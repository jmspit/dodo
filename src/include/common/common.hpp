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
 * @file common.hpp
 * Includes network headers and implements network::initLibrary() and network::closeLibrary().
 */

#ifndef dodo_common_common_hpp
#define dodo_common_common_hpp

#include <common/exception.hpp>
#include <common/puts.hpp>
#include <common/systemerror.hpp>
#include <common/util.hpp>


namespace dodo {

  /**
   * Common and utility interfaces.
   */
  namespace common {

    /**
     * Initialize the common library.
     */
    void initLibrary() {
    }

    /**
     * Close the common library.
     */
    void closeLibrary() {
    }

  }

}

#endif