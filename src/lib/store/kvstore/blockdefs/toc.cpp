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
 * @file toc.cpp
 * Implements the dodo::store::kvstore::TOC class.
 */

#include <common/exception.hpp>
#include <store/kvstore/blockdefs/toc.hpp>
#include <store/kvstore/kvstore.hpp>

#include <cstring>
#include <map>

namespace dodo::store::kvstore {

  void TOC::init( BlockId id, BlockId lowest ) {
    block_->block_header.init( store_->getBlockSize(), id, btTOC );
    block_->lowest = lowest;
    block_->highest = lowest;
    block_->next_toc = 0;
  }

  uint64_t TOC::getMaxEntries() const {
    return (store_->getBlockSize() - sizeof( BlockDef ) + sizeof( BlockType ))/sizeof( BlockType );
  }

  void TOC::setEntry( BlockId id, BlockType type ) {
    uint64_t entryid = 0;
    entryid = id - block_->lowest;
    if ( id >= block_->lowest && id <= block_->lowest + getMaxEntries() ) {
      BlockType* bt = &(block_->entry) + entryid;
      *bt = type;
      if ( id > block_->highest ) block_->highest = id;
    } else throw_Exception( "TOC entry " << id << " out of block range " << block_->lowest << " to " << block_->lowest + getMaxEntries() );
  }

  bool TOC::analyze( std::ostream &os ) {
    outputHeader( os, "TOC" );

    bool ok = true;
    Tester tester( os );
    ok = tester.test( common::Puts() << "blockid " << block_->block_header.blockid,
      [this](){ return block_->block_header.blockid > 0; } );

    ok = tester.test( common::Puts() << "crc32 0x" << common::Puts::hex() << block_->block_header.calcCRC32( store_->getBlockSize() ) << common::Puts::dec(),
      [this](){ return block_->block_header.verifyCRC32( store_->getBlockSize() ) ; } );

    os << "blockid range " << block_->lowest << " to " << block_->highest << endl;
    std::map<BlockType,size_t> type_histo;
    for ( BlockId i = block_->lowest; i <= block_->highest; i++ ) {
      BlockType* bt = &(block_->entry) + i;
      //os << "  blockid " << i << " type " << *bt << endl;
      type_histo[ *bt ]++;
    }
    for ( auto i = type_histo.begin(); i != type_histo.end(); i++ ) {
      switch( i->first ) {
        case btFileHeader:
          os << "#FileHeader " << i->second << std::endl;
          break;
        case btTOC:
          os << "#TOC " << i->second << std::endl;
          break;
        case btFree:
          os << "#Free " << i->second <<
          " " << i->second*store_->getBlockSize() << " bytes unused" << std::endl;
          break;
        case btData:
          os << "#Data " << i->second << std::endl;
          break;
        case btIndexLeaf:
          os << "#IndexLeaf " << i->second << std::endl;
          break;
        case btIndexTree:
          os << "#btIndexTree " << i->second << std::endl;
          break;
      }
    }
    return ok;
  }

}