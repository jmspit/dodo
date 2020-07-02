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
#include "common/exception.hpp"
#include "threads/mutex.hpp"

#ifndef common_config_hpp
#define common_config_hpp

namespace dodo::common {

  /**
   * Throws an Exception, passes __FILE__ and __LINE__ to constructor, and includes the config file path.
   * @param what The exception message as std::string.
   */
  #define throw_ConfigException( what ) throw dodo::common::Exception( __FILE__, __LINE__, \
          dodo::common::Puts() << dodo::common::Config::getConfig()->getPath() << " : " what )

  /**
   * Singleton class representing the deployment configuration, combining deployment constants from the configuration
   * file, environment variables (whose names start with the appname) and some operating system quantities useful to
   * developers, like the CPU count or an imposed memory limit.
   *
   * Configuration files are YAML files. Configuration values may use the encryption format of the DataCrypt
   * interface, so that secrets can safely be stored in YAML configuration files.
   *
   * @code
   * service:
   *   fqdn: remote.server.org
   *   port: 1827
   *   client_id: ENC[]
   *   client_secret: ENC[]
   * @endcode
   *
   * Each Application will have only one configuration file, and its location must be known when the singleton
   * Config instance is created, developers need to call initialize() first. All subsequent calls to getConfig() will
   * return the same pointer without the need to specify the configuration file path each time.
   *
   *
   * The Config class exposes the configuration data as const YAML::Node reference. So to modify or
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
       * Disallow the copy constructor as this is a singleton.
       */
      Config( const Config& ) = delete;

      /**
       * Disallow assignment from another Config, as this is a singleton.
       */
      Config& operator=( const Config& ) = delete;

      /**
       * Initialize the singleton. Once initialize is called, subsequent calls
       * to getConfig() will return the same pointer.
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
       * Re-read the configuration file. This call locks the internal getMutex().
       */
      void readConfig();

      /**
       * Write out the configuration file. This call locks the internal getMutex().
       */
      void writeConfig();

      /**
       * Return a reference to the configuration YAML contents. Callers must serialize access by locking
       * the getMutex() Mutex (or use a threads::Mutexer on it).
       * @return The YAML::Node reference to the YAML root.
       */
      YAML::Node& getYAML() { return yaml_; }

      /**
       * Return a reference to the serializing threads::Mutex.
       * @return A reference to the internal threads::Mutex.
       */
      threads::Mutex& getMutex() { return mutex_; }

      /**
       * Return the path of the configuration file.
       * @return The path of the configuration file.
       */
      std::string getPath() const { return path_; }

      /**
       * Return the application name.
       * @return the application name.
       */
      std::string getAppName() const { return yaml_["dodo"]["common"]["application"]["name"].as<std::string>(); }

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
       * The Mutex to serialize configuration data access.
       */
      static threads::Mutex mutex_;

    /**
     * Application::~Application() will destruct this singleton
     */
    friend class Application;
  };

}

#endif