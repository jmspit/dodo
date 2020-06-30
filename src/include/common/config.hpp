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

#ifndef common_config_hpp
#define common_config_hpp

namespace dodo::common {

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
   * The Config class is thread-safe as it serializes modifying operations internally.
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
       * Re-read the configuration file.
       */
      void readConfig();

      /**
       * Write out the configuration file.
       */
      void writeConfig();

      /**
       * Return a reference to the configuration YAML contents.
       * @return The YAML::Node reference to the YAML root.
       */
      YAML::Node& getYAML() { return yaml_; }

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
  };

}

#endif