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
 * @file config.hpp
 * Defines the dodo::common::Config class.
 */

#include <map>
#include <string>

#include <yaml-cpp/yaml.h>
#include <common/bytes.hpp>
#include <common/exception.hpp>
#include <threads/mutex.hpp>

#ifndef common_config_hpp
#define common_config_hpp

namespace dodo::common {

  /**
   * Singleton interface to a (read-only) deployment configuration, combining data from the deployment configuration
   * file, environment variables and some operating system attributes such as the CPU count, RAM and virtual memory size.
   *
   * As multiple threads may be reading and modifying the configuration data, access must be serialized. However, as the
   * data is exposed as a reference to a YAML::Node, this class cannot control serialization transparently. Developers
   * know when they are done reading or writing the configuration data. For example
   *
   * @code
   * {
   *   threads::Mutexer( getConfig()->getMutex() );
   *   const std::string username = config["username"].as<std::string>();
   * }
   * // Mutexer is out of scope, destructed and the getConfig()->getMutex() lock is released.
   * @endcode
   */
  class Config {
    public:

      /**
       * Used by getValue, enable use of list initializers as path specification, eg
       * @code
       * std::string s = getvalue<std::string>( { "level1","level2","level2"} )
       * @endcode
       * on this YAML
       * @code
       * level1:
       *   level2:
       *     level3: foo
       * @endcode
       * would set s to "foo". A KeyPath cannot index into arrays.
       */
      typedef std::list<std::string> KeyPath;

      /** The dodo root node */
      static const Config::KeyPath config_dodo;

      /** The dodo.common node */
      static const Config::KeyPath config_dodo_common;

      /** The dodo.common.application node */
      static const Config::KeyPath config_dodo_common_application;

      /** The dodo.common.application.name node */
      static const Config::KeyPath config_dodo_common_application_name;

      /** The dodo.common.application.secret node */
      static const Config::KeyPath config_dodo_common_application_secret;

      /** The dodo.common.application.secret.file node */
      static const Config::KeyPath config_dodo_common_application_secret_file;

      /** The dodo.common.application.secret.env node */
      static const Config::KeyPath config_dodo_common_application_secret_env;

      /** The dodo.common.logger node */
      static const Config::KeyPath config_dodo_common_logger;

      /** The dodo.common.logger.console node */
      static const Config::KeyPath config_dodo_common_logger_console;

      /** The dodo.common.logger.console.level node */
      static const Config::KeyPath config_dodo_common_logger_console_level;

      /** The dodo.common.logger.file node */
      static const Config::KeyPath config_dodo_common_logger_file;

      /** The dodo.common.logger.file.level node */
      static const Config::KeyPath config_dodo_common_logger_file_level;

      /** The dodo.common.logger.file.directory node */
      static const Config::KeyPath config_dodo_common_logger_file_directory;

      /** The dodo.common.logger.file.max-size-mib node */
      static const Config::KeyPath config_dodo_common_logger_file_max_size_mib;

      /** The dodo.common.logger.file.max-file-trail node */
      static const Config::KeyPath config_dodo_common_logger_file_max_file_trail;

      /** The dodo.common.logger.syslog node */
      static const Config::KeyPath config_dodo_common_logger_syslog;

      /** The dodo.common.logger.syslog.level node */
      static const Config::KeyPath config_dodo_common_logger_syslog_level;

      /** The dodo.common.logger.syslog.facility node */
      static const Config::KeyPath config_dodo_common_logger_syslog_facility;



      /**
       * Transform a keypath to a string with ":" as seperator between levels.
       * @param keypath The KeyPath to flatten.
       * @return The flattened keypath.
       */
      static std::string flattenKeyPath( const KeyPath& keypath );

      /**
       * Disallow the copy constructor as this is a singleton.
       */
      Config( const Config& ) = delete;

      /**
       * Disallow assignment from another Config, as this is a singleton.
       */
      Config& operator=( const Config& ) = delete;

      /**
       * Initialize the singleton. Once initialize is called, subsequent calls
       * to getConfig() will return the same pointer as returned by this function.
       * @param path The path to the configuration file.
       * @return a pointer to the Config singleton.
       */
      static Config* initialize( const std::string path );

      /**
       * return the singleton.
       * @return a pointer to the Config instance.
       * @throw Exception when
       */
      static Config* getConfig();

      /**
       * Return the path of the configuration file.
       * @return The path of the configuration file.
       */
      std::string getConfigPath() const { return path_; }

      /**
       * Read the configuration file.
       */
      void readConfig();


      /**
       * Return the application name.
       * @return the application name.
       */
      std::string getAppName() const { return getValue<std::string>( {"dodo","common","application","name"} ); }

      /**
       * Get the application secret.
       * @return The secret.
       */
      std::string getSecret() const { return secret_; }

      /**
       * Get the value at keypath.
       * @param keypath The KeyPath to get the value for.
       * @return The value.
       */
      template <typename T> T getValue( const KeyPath &keypath ) const {
        YAML::Node ref = Clone( yaml_ );
        for( const auto &k : keypath ) {
          if ( ! ref[k] ) throw_Exception( "key " << flattenKeyPath(keypath) << " not found in " << path_ );
          ref = ref[k];
        }
        return ref.as<T>();
      }

      /**
       * Return true if the KeyPath exists.
       * @param keypath The KeyPath to check.
       * @return True if the keypath exists.
       */
      bool exists( const KeyPath &keypath ) const {
        YAML::Node ref = Clone( yaml_ );
        for( const auto &k : keypath ) {
          if ( ! ref[k] ) return false;
          ref = ref[k];
        }
        return true;
      }

      /**
       * Get the decrypted value from KeyPath - decrypted using the secret from getSecret().
       * @param keypath The KeyPath to the the value for.
       * @param decrypted Receives the decrypted Bytes.
       */
      void getDecryptedValue( const KeyPath &keypath, Bytes &decrypted );

    protected:

      /**
       * Constructor called by initialize().
       * @param path The path to the configuration file.
       */
      Config( const std::string path );

      /**
       * Destructor.
       */
      virtual ~Config() {}

      /**
       * Check for required elements in the config file.
       */
      void checkConfig();

      /**
       * The singleton pointer.
       */
      static Config* config_;

      /**
       * The path to the configuration file.
       */
      static std::string path_;

      /**
       * The root YAML node.
       */
      YAML::Node yaml_;

      /**
       * The encryption secret / key.
       */
      std::string secret_;

    /**
     * Application::~Application() will destruct this singleton
     */
    friend class Application;
  };

}

#endif