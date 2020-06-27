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

#ifndef common_config_hpp
#define common_config_hpp

namespace dodo::common {

  /**
   * Singleton class representing the deployment configuration, combining user defined constant, environment
   * variables and some operating system defined values of use to developers, like the CPU count and
   * available RAM.
   */
  class Config {
    public:

      /**
       * return the singleton.
       * @return a pointer to the Config instance.
       */
      static Config* getConfig();

    protected:

      /**
       * The singleton pointer.
       */
      static Config* config_;
  };

}

#endif