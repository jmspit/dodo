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
 * @file common.cpp
 * Implements dodo::store::KVStore common things
 */

#include <common/exception.hpp>
#include <store/kvstore/blockdefs/toc.hpp>

#include <cstring>

namespace dodo::store::kvstore {

  void TOC::init( BlockId id ) {
    block_->block_header.init( blocksize_, id, btTOC );
    block_->lowest = 0;
    block_->highest = 0;
    block_->next_toc = 0;
    block_->entries = 0;
  }
    
  void TOC::setEntry( BlockId id, BlockType type ) {
    uint64_t entryid = 0;
    if ( block_->entries ) entryid = id - block_->lowest;
    if ( entryid <= getMaxEntries() ) {
      BlockType* bt = &(block_->entry) + entryid;
      *bt = type;
      if ( block_->lowest == 0 || id < block_->lowest ) block_->lowest = id;
      if ( block_->highest == 0 || id > block_->highest ) block_->highest = id;
    } else throw_Exception( "TOC entry out of block range" );
  }

}