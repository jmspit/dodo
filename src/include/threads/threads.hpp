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
 * @file threads.hpp
 * Includes network headers and implements network::initLibrary() and network::closeLibrary().
 */

#ifndef threads_threads_hpp
#define threads_threads_hpp

#include <threads/mutex.hpp>
#include <threads/thread.hpp>

namespace dodo {

  /**
   * Interface for Thread programming.
   */
  namespace threads {

    /**
     * Initialize the dodo::threads library.
     */
    void initLibrary() {
    }

    /**
     * Close the dodo::threads library.
     */
    void closeLibrary() {
    }

  }

}

#endif