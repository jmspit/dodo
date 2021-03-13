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
 * @file persist.hpp
 * Defines the persist namespace.
 */

#ifndef dodo_persist_hpp
#define dodo_persist_hpp

#include <persist/sqlite/sqlite.hpp>
#include <persist/kvstore/kvstore.hpp>

namespace dodo {

  /**
   * Persistent storage API.
   *   - SQLite C++ API
   *   - Key-value store API backed by sqlite
   */
  namespace persist {
  }

}

#endif
