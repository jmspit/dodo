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
 * @file config.cpp
 * Implements the dodo::common::Config class.
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include <common/config.hpp>
#include <common/datacrypt.hpp>
#include <common/exception.hpp>
#include <common/util.hpp>

namespace dodo::common {

  const Config::KeyPath Config::config_dodo = {"dodo"};
  const Config::KeyPath Config::config_dodo_common = {"dodo","common"};
  const Config::KeyPath Config::config_dodo_common_application = {"dodo","common","application"};
  const Config::KeyPath Config::config_dodo_common_application_name = {"dodo","common","application","name"};
  const Config::KeyPath Config::config_dodo_common_application_secret = {"dodo","common","application","secret"};
  const Config::KeyPath Config::config_dodo_common_application_secret_file = {"dodo","common","application","secret","file"};
  const Config::KeyPath Config::config_dodo_common_application_secret_env = {"dodo","common","application","secret","env"};
  const Config::KeyPath Config::config_dodo_common_logger = {"dodo","common","logger"};
  const Config::KeyPath Config::config_dodo_common_logger_console = {"dodo","common","logger","console"};
  const Config::KeyPath Config::config_dodo_common_logger_console_level = {"dodo","common","logger","console","level"};
  const Config::KeyPath Config::config_dodo_common_logger_file = {"dodo","common","logger","file"};
  const Config::KeyPath Config::config_dodo_common_logger_file_level = {"dodo","common","logger","file","level"};
  const Config::KeyPath Config::config_dodo_common_logger_file_directory = {"dodo","common","logger","file","directory"};
  const Config::KeyPath Config::config_dodo_common_logger_file_max_size_mib = {"dodo","common","logger","file","max-size-mib"};
  const Config::KeyPath Config::config_dodo_common_logger_file_max_file_trail = {"dodo","common","logger","file","max-file-trail"};
  const Config::KeyPath Config::config_dodo_common_logger_syslog = {"dodo","common","logger","syslog"};
  const Config::KeyPath Config::config_dodo_common_logger_syslog_level = {"dodo","common","logger","syslog","level"};
  const Config::KeyPath Config::config_dodo_common_logger_syslog_facility = {"dodo","common","logger","syslog","facility"};

  Config* Config::config_ = nullptr;

  std::string Config::path_ = "";

  Config::Config( const std::string path ) {
    path_ = path;
    readConfig();
  }

  Config* Config::initialize( const std::string path ) {
    if ( config_ ) throw_Exception( "calling initialize on an existing Config singleton" );
    config_ = new Config( path );
    return config_;
  }

  Config* Config::getConfig() {
    if ( config_ == nullptr ) throw_Exception( "null singleton - call initialize before getConfig" );
    return config_;
  }

  void Config::checkConfig() {
    try {
      if ( exists(config_dodo) ) {
        if ( exists(config_dodo_common) ) {
          if ( exists(config_dodo_common_application) ) {
            if ( !exists(config_dodo_common_application_name) ) throw_Exception( flattenKeyPath(config_dodo_common_application_name)
                                                                                   << " node missing" );
            if ( exists(config_dodo_common_application_secret) ) {
              if ( exists(config_dodo_common_application_secret_file) ) {
                secret_ = common::fileReadString( getValue<std::string>(config_dodo_common_application_secret_file) );
              } else if ( exists(config_dodo_common_application_secret_env) ) {
                char* c = secure_getenv( getValue<std::string>(config_dodo_common_application_secret_env).c_str() );
                if ( c ) secret_ = c;
                else throw_Exception( "ENV var " <<  getValue<std::string>( config_dodo_common_application_secret_env ) << " does not exist" );
              } else throw_Exception( flattenKeyPath( config_dodo_common_application_secret ) << " must have one of 'file' or 'env'" );
            }
          } else throw_Exception( flattenKeyPath( config_dodo_common_application ) << " node missing" );
          if ( exists(config_dodo_common_logger) ) {

            if ( exists(config_dodo_common_logger_console) ) {
              if ( exists(config_dodo_common_logger_console_level) ) {
              } else throw_Exception( flattenKeyPath(config_dodo_common_logger_console_level) << " node missing" );
            } else throw_Exception( flattenKeyPath(config_dodo_common_logger_console) << " node missing" );

            if ( exists(config_dodo_common_logger_file) ) {
              if ( exists(config_dodo_common_logger_file_level) ) {
              } else throw_Exception( flattenKeyPath(config_dodo_common_logger_file_level) << " node missing" );
              if ( exists(config_dodo_common_logger_file_directory) ) {
              } else throw_Exception( flattenKeyPath(config_dodo_common_logger_file_directory) << " node missing" );
            }

            if ( exists(config_dodo_common_logger_syslog) ) {
              if ( exists(config_dodo_common_logger_syslog_level) ) {
              } else throw_Exception( flattenKeyPath(config_dodo_common_logger_syslog_level) << " node missing" );
            }
          } else throw_Exception( flattenKeyPath(config_dodo_common_logger) << " node missing" );
        } else throw_Exception( flattenKeyPath(config_dodo_common) << " node missing" );
      } else throw_Exception( flattenKeyPath(config_dodo) << " node missing" );
    }
    catch ( const std::exception &e ) {
      throw_Exception( "Config file '" << path_ << "' content error: " << e.what() );
    }
  }

  void Config::readConfig() {
    try {
      yaml_ = YAML::LoadFile(path_);
    }
    catch ( const std::exception &e ) {
      throw_Exception( "Config file '" << path_ << "' parse error: " << e.what() );
    }
    checkConfig();
  }

  std::string Config::flattenKeyPath( const KeyPath& keypath ) {
    std::stringstream ss;
    for ( const auto &k : keypath ) {
      if ( ss.str().size() ) ss << "::";
      ss << k;
    }
    return ss.str();
  }

  void Config::getDecryptedValue( const KeyPath &keypath, Bytes &decrypted ) {
    YAML::Node ref = Clone( yaml_ );
    for( const auto &k : keypath ) {
      if ( ! ref[k] ) throw_Exception( "key " << flattenKeyPath(keypath) << " not found in " << path_ );
      ref = ref[k];
    }
    DataCrypt::decrypt( getSecret(), ref.as<std::string>(), decrypted );
  }


}