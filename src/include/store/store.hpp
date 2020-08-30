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
 * @file store.hpp
 * Includes store headers.
 */

#ifndef store_store_hpp
#define store_store_hpp

#include <store/kvstore/kvstore.hpp>
#include <store/kvstore/blockdefs/common.hpp>
#include <store/kvstore/blockdefs/data.hpp>
#include <store/kvstore/blockdefs/index.hpp>
#include <store/kvstore/blockdefs/toc.hpp>

namespace dodo {

  /**
   * Interface to persistent storage.
   */
  namespace store {

    /**
     * Interface to the key-value store
     */
    namespace kvstore {
    }

  }

}

#endif