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
 * @file data.cpp
 * Implements the dodo::store::kvstore::Data class.
 */

#include <store/kvstore/blockdefs/data.hpp>
#include <store/kvstore/kvstore.hpp>

#include <cassert>
#include <climits>
#include <cstring>

namespace dodo::store::kvstore {

  void Data::init( BlockId id ) {
    block_->block_header.init( store_->getBlockSize(), id, btData );
    block_->num_rows = 0;
    block_->low_data_offset = store_->getBlockSize();
  }

  BlockSize Data::getFreeSpace() const {
    return block_->low_data_offset -
           static_cast<BlockSize>( sizeof(BlockDef) ) -
           ( (block_->num_rows+1) * static_cast<BlockSize>( sizeof(RowEntry) ) );
  }

  Data::RowEntry* Data::getRowEntryByIndex( RowId index ) const {
    assert( index < block_->num_rows );
    RowEntry *p = reinterpret_cast<RowEntry*>( &(block_->first_entry) );
    return p + index;
  }

  Data::RowEntry* Data::getRowEntryByRowId( RowId rowid ) const {
    assert( block_->num_rows != 0 );
    if ( !block_->num_rows ) return nullptr;
    RowId found = matchSearch( rowid, 0, block_->num_rows -1 );
    assert( found != InvalidRowId );
    if ( found == InvalidRowId ) return nullptr;
    return getRowEntryByIndex( found );
  }

  RowId Data::insertSearch( RowId rowid, RowId low, RowId high ) const {
    assert( high < block_->num_rows );
    if ( high <= low )
      return (rowid > getRowEntryByIndex(low)->rowid) ? (low + 1): low;
    RowId mid = (low + high)/2;
    if( rowid == getRowEntryByIndex(mid)->rowid )
      return mid+1;
    if( rowid > getRowEntryByIndex(mid)->rowid )
      return insertSearch( rowid, mid+1, high);
    if ( mid > 0 ) return  insertSearch( rowid, low, mid-1 ); else return mid;
  }

  RowId Data::matchSearch( RowId rowid, RowId low, RowId high ) const {
    assert( high < block_->num_rows );
    if ( high >= low ) {
      RowId mid = low + (high - low) / 2;
      if ( rowid == getRowEntryByIndex(mid)->rowid ) return mid;
      if ( rowid > getRowEntryByIndex(mid)->rowid )
        return matchSearch( rowid, mid+1, high );
      if ( mid > 0 ) return matchSearch( rowid, low, mid-1 ); else return InvalidRowId;
    }
    return InvalidRowId;
  }

  RowId Data::allocateRow( BlockSize size ) {
    assert( size <= getFreeSpace() );

    RowId new_rowid = getFreeRowId();
    RowId before_index = 0;
    if ( block_->num_rows > 0 ) before_index = insertSearch( new_rowid, 0, block_->num_rows-1 );
    RowEntry* p = getFirstRowEntry();

    if ( block_->num_rows > 0 && before_index < block_->num_rows ) {
      p += before_index;
      BlockSize bd_sz = static_cast<BlockSize>(sizeof(BlockDef));
      BlockSize re_sz = static_cast<BlockSize>(sizeof(RowEntry));
      std::memmove( voidPtr( bd_sz + before_index * re_sz ),
                    voidPtr( bd_sz + before_index * re_sz - re_sz ),
                    (block_->num_rows - before_index) * re_sz );
    } else  {
      p += (block_->num_rows);
    }
    p->rowid = new_rowid;
    p->offset = block_->low_data_offset - size;
    p->size = size;
    p->next_block = 0;
    p->next_rowid = 0;

    block_->num_rows++;
    block_->low_data_offset = p->offset;
    return p->rowid;
  }

  RowId Data::allocateRow( const OctetArray& src ) {
    assert( static_cast<size_t>(src.size) < (UINT_MAX-1) );
    RowId new_row = allocateRow( static_cast<BlockSize>(src.size) );
    RowEntry* new_entry = getRowEntryByRowId( new_row );
    std::memcpy( voidPtr( new_entry->offset ), src.array, new_entry->size );
    return new_row;
  }

  bool Data::getRowData( RowId rowid, common::OctetArray& data ) const {
    RowEntry* entry = getRowEntryByRowId( rowid );
    if ( entry ) {
      data.reserve( entry->size );
      std::memcpy( data.array, voidPtr(entry->offset), entry->size );
      return true;
    } else return false;
  }

  void Data::releaseRow( RowId rowid ) {
    RowEntryIndexPair pair = matchSearch( rowid );
    if ( pair.entry != nullptr ) {
      BlockSize target_sz = pair.entry->size; // as this will be overwritten
      BlockSize target_offset = pair.entry->offset;
      if ( target_offset > block_->low_data_offset ) {
        // there is a gap as the released row is not the lowest row
        //std::cout << "sz=" << target_sz << std::endl;
        //std::cout << "src=" << block_->low_data_offset << std::endl;
        //std::cout << "dst=" << block_->low_data_offset + target_sz << std::endl;
        //std::cout << "len=" << target->offset - block_->low_data_offset << std::endl;
        std::memmove( voidPtr( block_->low_data_offset + target_sz ),
                      voidPtr( block_->low_data_offset ),
                      target_offset - block_->low_data_offset );
        // zero the freed zone
        std::memset( voidPtr( block_->low_data_offset ), 0, target_sz );
      }
      // and a gap in the RowEntry list
      BlockSize bd_sz = static_cast<BlockSize>(sizeof(BlockDef));
      BlockSize re_sz = static_cast<BlockSize>(sizeof(RowEntry));
      //std::cout << "src=" << bd_sz + index * re_sz << std::endl;
      //std::cout << "dst=" << bd_sz + index * re_sz - re_sz << std::endl;
      //std::cout << "len=" << (block_->num_rows - index) * re_sz << std::endl;
      std::memmove( voidPtr( bd_sz + pair.index * re_sz - re_sz ),
                    voidPtr( bd_sz + pair.index * re_sz ),
                    (block_->num_rows - pair.index) * re_sz );
      // fix num_rows
      block_->num_rows--;
      // zero the freed zone
      std::memset( voidPtr( bd_sz - re_sz + block_->num_rows * re_sz ), 0, re_sz );
      // fix the offsets
      /**
       * @todo - this is O(block_->num_rows) as there is no sorted list on offfset
       * to limit the adjustment. the loop content is tiny, and this is a tradeoff for gap compresison
       * under row deletion - unless we also maintain a list sorted on offset per rowid, which in turn requires
       * an anchor point - but there are already two lists here, making it cumbersome.
       */
      RowEntry* p = getFirstRowEntry();
      for ( RowId r = 0; r < block_->num_rows; r++, p++ ) {
        if ( p->offset < target_offset ) p->offset += target_sz;
      }
      block_->low_data_offset += target_sz;
    }
  }

  void Data::setNextRow( RowId rowid, BlockId next_blockid, RowId next_rowid ) {
    RowEntry* p = getFirstRowEntry();
    for ( RowId r = 0; r < block_->num_rows; r++, p++ ) {
      if ( p->rowid == rowid ) {
        p->next_block = next_blockid;
        p->next_rowid = next_rowid;
        break;
      }
    }
  }

  bool Data::analyze( std::ostream &os ) {
    bool ok = true;
    outputHeader( os, "Data" );

    Tester tester( os );
    ok = tester.test( common::Puts() << "blockid " << block_->block_header.blockid,
      [this](){ return block_->block_header.blockid >= 3; } );

    ok = tester.test( common::Puts() << "blocktype " << block_->block_header.blocktype,
      [this](){ return block_->block_header.blocktype == BlockType::btData; } );

    ok = tester.test( common::Puts() << "crc32 0x" << common::Puts::hex() << block_->block_header.calcCRC32( store_->getBlockSize() ) << common::Puts::dec(),
      [this](){ return block_->block_header.verifyCRC32( store_->getBlockSize() ) ; } );

    os << "num_rows " << block_->num_rows << std::endl;
    os << "low data " << block_->low_data_offset << std::endl;
    os << "free space " << getFreeSpace() << std::endl;

    const RowEntry* p = getFirstConstRowEntry();
    for ( RowId r = 0; r < block_->num_rows; r++, p++ ) {
      os << "rowid " << p->rowid << " size " << p->size << " offset " << p->offset;
      common::OctetArray data;
      if ( getRowData( p->rowid, data ) ) {
        os << " '" << data.asString() << "'";
      }
      os << endl;
    }
    return ok;
  }

  Data::RowEntry* Data::getFirstRowEntry() const { return reinterpret_cast<RowEntry*>( &(block_->first_entry) ); }

  const Data::RowEntry* Data::getFirstConstRowEntry() const { return reinterpret_cast<const RowEntry*>( &(block_->first_entry) ); }

  RowId Data::findFreeRowId( RowId low, RowId high ) const {
    //cout << "low=" << low << " high=" << high << endl;
    if ( high >= low ) {
      RowId mid = low + ( high - low)/2;
      //cout << "mid=" << mid << " rmid=" << getRowEntryByIndex(mid)->rowid << endl;
      if ( high == low ) return mid + (mid == getRowEntryByIndex(mid)->rowid);
      if ( getRowEntryByIndex(mid)->rowid > mid ) {
        if ( mid > 0 ) return findFreeRowId( low, mid-1 ); else return getRowEntryByIndex(mid)->rowid-1;
      }
      else return findFreeRowId( mid+1, high );
    }
    return low;
  }

  RowId Data::getFreeRowId() const {
    if ( block_->num_rows == 0 ) return 0;
    const RowEntry* max_entry = getFirstConstRowEntry();
    max_entry += block_->num_rows - 1;
    if ( block_->num_rows < max_entry->rowid + 1 ) {
      return findFreeRowId( 0, block_->num_rows - 1 );
    } else {
      return block_->num_rows;
    }
  }

}