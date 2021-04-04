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
 * @file fifoqueue.hpp
 * Defines the FIFOQueue class.
 */

#ifndef dodo_kvstore_hpp
#define dodo_kvstore_hpp

#include <filesystem>
#include <sqlite3.h>

/**
 * First-In-First-Out queue backed by SQLite3.
 */
class FIFOQueue {

  public:
    FIFOQueue( const std::filesystem::path &path );
    ~FIFOQueue();

    void produce();

    void consume();

};

#endif