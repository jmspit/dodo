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
 * @file indextree.cpp
 * Implements the dodo::store::kvstore::IndexTree class.
 */

#include <store/kvstore/blockdefs/indextree.hpp>

#include <cstring>

namespace dodo::store::kvstore {

  void IndexTree::init( BlockId id ) {
    block_->block_header.init( blocksize_, id, btIndexTree );
  }

}