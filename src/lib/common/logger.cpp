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

#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/exception.hpp"
#include "common/util.hpp"

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
    YAML::Node &yaml = config.getConfig()->getYAML();
    levels_[Console] = StringAsLogLevel( yaml["dodo"]["common"]["logger"]["console"]["level"].as<std::string>() );

    if ( yaml["dodo"]["common"]["logger"]["file"] ) {
      destinations_  = destinations_ | File;
      getFileParams(config);
    }

    if ( yaml["dodo"]["common"]["logger"]["syslog"] ) {
      destinations_  = destinations_ | Syslog;
      if ( yaml["dodo"]["common"]["logger"]["syslog"]["facility"] ) {
        syslog_params_.facility = yaml["dodo"]["common"]["logger"]["syslog"]["facility"].as<int>();
      } else syslog_params_.facility = 1;

      levels_[Syslog] = StringAsLogLevel( yaml["dodo"]["common"]["logger"]["syslog"]["level"].as<std::string>() );
      //openlog( config.getAppName().c_str(), LOG_CONS | LOG_PID | LOG_NDELAY, syslog_params_.facility );
    }

  }

  Logger::~Logger() {
    // wait until other trades have released. they may be writing.
    threads::Mutexer lock( mutex_ );
    file_params_.file.close();
    //closelog();
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

  void Logger::log( LogLevel level, const std::string message ) {
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

  int Logger::mapLeveltoSyslog( LogLevel level ) {
    switch ( level ) {
      case LogLevel::Fatal : return 2;
      case LogLevel::Error : return 3;
      case LogLevel::Warning : return 4;
      case LogLevel::Info :
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

      std::set<string> trail;
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
    YAML::Node &yaml = config.getConfig()->getYAML();
    levels_[File] = StringAsLogLevel( yaml["dodo"]["common"]["logger"]["file"]["level"].as<std::string>() );
    file_params_.directory = yaml["dodo"]["common"]["logger"]["file"]["directory"].as<std::string>();

    if ( yaml["dodo"]["common"]["logger"]["file"]["max-size-mib"] )
      file_params_.max_size_mib = yaml["dodo"]["common"]["logger"]["file"]["max-size-mib"].as<size_t>();
    else
      file_params_.max_size_mib = 10;

    if ( yaml["dodo"]["common"]["logger"]["file"]["max-file-trail"] )
      file_params_.max_file_trail = yaml["dodo"]["common"]["logger"]["file"]["max-file-trail"].as<size_t>();
    else
      file_params_.max_file_trail = 4;

    if ( !directoryExists( file_params_.directory ) )
      throw_Exception( Puts() << "directory '" << file_params_.directory << "' does not exist" );
    if ( !directoryWritable( file_params_.directory ) )
      throw_Exception( Puts() << "directory '" << file_params_.directory << "' does not allow write access" );
    file_params_.active_log = file_params_.directory + "/" + config.getAppName() + ".log";
    file_params_.file.open( file_params_.active_log, std::ofstream::out | std::ofstream::app );
    file_params_.filesize = getFileSize( file_params_.active_log );
    if ( !file_params_.file.good() )
      throw_Exception( Puts() << "failed to open '" << file_params_.active_log << "' for writing" );
  }

  std::string Logger::formatMessage( LogLevel level, const std::string message ) {
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
        case LogLevel::Debug : return "DBG";
        case LogLevel::Trace : return "TRC";
      }
    } else {
      switch ( level ) {
        case LogLevel::Fatal : return "fatal";
        case LogLevel::Error : return "error";
        case LogLevel::Warning : return "warning";
        case LogLevel::Info : return "info";
        case LogLevel::Debug : return "debug";
        case LogLevel::Trace : return "trace";
      }
    }
    return ""; // never reaches but g++ complains.
  }

}