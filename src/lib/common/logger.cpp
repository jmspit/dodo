/*
 * This file is part of the dodo library (https://github.com/jmspit/dodo).
 * Copyright (c) 2019 Jan-Marten Spit.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file logger.cpp
 * Implements the dodo::common::Logger class.
 */

#include <fstream>
#include <iostream>

#include <common/config.hpp>
#include "common/logger.hpp"
#include <common/exception.hpp>
#include <common/util.hpp>

#include <yaml-cpp/yaml.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <filesystem>
#include <regex>
#include <syslog.h>
#include <sys/syscall.h>

namespace dodo::common {

  threads::Mutex Logger::mutex_;

  Logger* Logger::logger_ = nullptr;

  Logger::Logger( const Config& config ) {
    rotate_throttle_counter_ = 0;
    destinations_ = Console;
    char hostname[256];
    gethostname( hostname, 256 );
    hostname_ = hostname;
    threads::Mutexer lock( mutex_ );
    levels_[Console] = StringAsLogLevel( config.getConfig()->getValue<std::string>( Config::config_dodo_common_logger_console_level ) );

    if ( config.getConfig()->exists( Config::config_dodo_common_logger_file ) ) {
      destinations_  = destinations_ | File;
      getFileParams(config);
    }

    if ( config.getConfig()->exists( Config::config_dodo_common_logger_syslog ) ) {
      destinations_  = destinations_ | Syslog;
      if ( config.getConfig()->exists( Config::config_dodo_common_logger_syslog_facility ) ) {
        syslog_params_.facility = config.getConfig()->getValue<int>( Config::config_dodo_common_logger_syslog_facility );
      } else syslog_params_.facility = 1;
      levels_[Syslog] = StringAsLogLevel( config.getConfig()->getValue<std::string>( Config::config_dodo_common_logger_syslog_level ) );
      if ( levels_[Syslog] > LogLevel::Info ) throw_Exception( "syslog LogLevel cannot be higher than info" );
    }

  }

  Logger::~Logger() {
    // wait until other threads have released. they may be writing.
    threads::Mutexer lock( mutex_ );
    file_params_.file.close();
  }

  Logger* Logger::initialize( const Config& config ) {
    if ( logger_ ) throw_Exception( "calling initialize on an existing Logger singleton" );
    logger_ = new Logger( config );
    return logger_;
  }

  Logger* Logger::getLogger() {
    if ( !logger_ ) throw_Exception( "null singleton - call initialize before getLogger" );
    return logger_;
  }

  void Logger::log( LogLevel level, const std::string &message ) {
    threads::Mutexer lock( mutex_ );
    std::string entry = "";

    if ( level <= levels_[Console] ) {
      entry = formatMessage( level, message );
      std::cout << entry << std::endl;
    }

    if ( destinations_ | File && level <= levels_[File] ) {
      checkRotate();
      if ( entry.length() == 0 ) entry = formatMessage( level, message );
      file_params_.file << entry << std::endl;
      file_params_.filesize += entry.length() + 1;
    }

    if ( destinations_ | Syslog && level <= levels_[Syslog] && level <= LogLevel::Info ) {
      std::stringstream ss;
      ss << Config::getConfig()->getAppName() << "[" << syscall(SYS_gettid) << "]: ";
      ss << LogLevelAsString(level, true) << ": " << message;
      syslog( syslog_params_.facility | mapLeveltoSyslog(level), "%s", ss.str().c_str() );
    }
  }

  void Logger::fatal( const std::string &message ) {
    log( LogLevel::Fatal, message );
  }

  void Logger::error( const std::string &message ) {
    log( LogLevel::Error, message );
  }

  void Logger::warning( const std::string &message ) {
    log( LogLevel::Warning, message );
  }

  void Logger::info( const std::string &message ) {
    log( LogLevel::Info, message );
  }

  void Logger::statistics( const std::string &message ) {
    log( LogLevel::Statistics, message );
  }

  void Logger::debug( const std::string &message ) {
    log( LogLevel::Debug, message );
  }

  void Logger::trace( const std::string &message ) {
    log( LogLevel::Trace, message );
  }

  int Logger::mapLeveltoSyslog( LogLevel level ) {
    switch ( level ) {
      case LogLevel::Fatal : return 2;
      case LogLevel::Error : return 3;
      case LogLevel::Warning : return 4;
      case LogLevel::Info :
      case LogLevel::Statistics :
      case LogLevel::Debug :
      case LogLevel::Trace : return 6;
    }
    return 6;
  }

  void Logger::checkRotate() {
    if ( file_params_.filesize > file_params_.max_size_mib * 1024 * 1024 ) {
      file_params_.file.close();
      std::stringstream ss;
      struct timeval tv;
      gettimeofday( &tv, nullptr );
      ss << file_params_.active_log << "." << formatDateTimeUTC( tv );
      rename( file_params_.active_log.c_str(), ss.str().c_str() );
      file_params_.file.open( file_params_.active_log, std::ofstream::out | std::ofstream::app );
      file_params_.filesize = getFileSize( file_params_.active_log );

      std::set<std::string> trail;
      std::regex log_regex( "^" + file_params_.active_log + "\\..*" );
      for ( const auto & entry : std::filesystem::directory_iterator( file_params_.directory ) ) {
        if ( std::regex_match( entry.path().string(), log_regex ) ) {
          trail.insert( entry.path().string() );
        }
      }

      while ( trail.size() > file_params_.max_file_trail ) {
        std::string path = *trail.begin();
        unlink( path.c_str() );
        trail.erase( trail.begin() );
      }
    }
  }

  void Logger::getFileParams( const Config& config ) {
    levels_[File] = StringAsLogLevel( config.getConfig()->getValue<std::string>(Config::config_dodo_common_logger_file_level) );
    file_params_.directory = config.getConfig()->getValue<std::string>(Config::config_dodo_common_logger_file_directory);

    if ( config.getConfig()->exists(Config::config_dodo_common_logger_file_max_size_mib) )
      file_params_.max_size_mib = config.getConfig()->getValue<size_t>(Config::config_dodo_common_logger_file_max_size_mib);
    else
      file_params_.max_size_mib = 10;

    if ( config.getConfig()->exists(Config::config_dodo_common_logger_file_max_file_trail) )
      file_params_.max_file_trail = config.getConfig()->getValue<size_t>(Config::config_dodo_common_logger_file_max_file_trail);
    else
      file_params_.max_file_trail = 4;

    if ( !directoryExists( file_params_.directory ) )
      throw_Exception( "directory '" << file_params_.directory << "' does not exist" );
    if ( !directoryWritable( file_params_.directory ) )
      throw_Exception( "directory '" << file_params_.directory << "' not writable" );
    file_params_.active_log = file_params_.directory + "/" + config.getAppName() + ".log";
    file_params_.file.open( file_params_.active_log, std::ofstream::out | std::ofstream::app );
    file_params_.filesize = getFileSize( file_params_.active_log );
    if ( !file_params_.file.good() )
      throw_Exception( "failed to open '" << file_params_.active_log << "' for writing" );
  }

  std::string Logger::formatMessage( LogLevel level, const std::string &message ) {
    std::stringstream ss;
    struct timeval tv;
    gettimeofday( &tv, nullptr );
    ss << formatDateTimeUTC( tv ) << " ";
    ss << hostname_ << " ";
    ss << Config::getConfig()->getAppName() << "[" << syscall(SYS_gettid) << "] ";
    ss << LogLevelAsString( level, true ) << " ";
    ss << message;
    return ss.str();
  }

  Logger::LogLevel Logger::StringAsLogLevel( const std::string slevel ) {
    if ( slevel == "fatal" ) return LogLevel::Fatal;
    else if ( slevel == "error" ) return LogLevel::Error;
    else if ( slevel == "warning" ) return LogLevel::Warning;
    else if ( slevel == "info" ) return LogLevel::Info;
    else if ( slevel == "statistics" ) return LogLevel::Statistics;
    else if ( slevel == "debug" ) return LogLevel::Debug;
    else if ( slevel == "trace" ) return LogLevel::Trace;
    else return LogLevel::Info;
  }

  std::string  Logger::LogLevelAsString( Logger::LogLevel level, bool acronym ) {
    if ( acronym ) {
      switch ( level ) {
        case LogLevel::Fatal : return "FAT";
        case LogLevel::Error : return "ERR";
        case LogLevel::Warning : return "WRN";
        case LogLevel::Info : return "INF";
        case LogLevel::Statistics : return "STA";
        case LogLevel::Debug : return "DBG";
        case LogLevel::Trace : return "TRC";
      }
    } else {
      switch ( level ) {
        case LogLevel::Fatal : return "fatal";
        case LogLevel::Error : return "error";
        case LogLevel::Warning : return "warning";
        case LogLevel::Info : return "info";
        case LogLevel::Statistics : return "statistics";
        case LogLevel::Debug : return "debug";
        case LogLevel::Trace : return "trace";
      }
    }
    return ""; // never reaches but g++ complains.
  }

}