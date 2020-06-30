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
 * @file dodo.hpp
 * @brief Includes all dodo headers.
 */

#ifndef dodo_hpp
#define dodo_hpp

#include <buildenv.hpp>
#include <common/common.hpp>
#include <network/network.hpp>
#include <threads/threads.hpp>

/**
 * A C++ platform interface to lean linux services tailored for containerized deployment.
 */
namespace dodo {

  /**
   * Initialize the dodo library.
   * @see dodo::common::Application
   */
  void initLibrary() {
    common::initLibrary();
    threads::initLibrary();
    network::initLibrary();
  }

  /**
   * Close the dodo library.
   * @see dodo::common::Application
   */
  void closeLibrary() {
    network::closeLibrary();
    threads::closeLibrary();
    common::closeLibrary();
  }

}

#endif