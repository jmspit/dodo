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
 * @file data.hpp
 * Defines the dodo::store::kvstore::Data class.
 */

// vscode intellisense reports false positives
#if __INTELLISENSE__
#pragma diag_suppress 144
#pragma diag_suppress 147
#endif

#ifndef store_kvstore_blockdefs_data_hpp
#define store_kvstore_blockdefs_data_hpp

#include <common/octetarray.hpp>
#include <store/kvstore/blockdefs/common.hpp>

namespace dodo::store::kvstore {

  /**
   * A data block. A data block can contains ero or more rows, each assigned a unique RowId
   * within the block. Each row is tracked in a RowEntry list, sorted by RowId, its first element stored in BlockDef::first_entry.
   *
   * As the RowEntry list is sorted, a binary search locates entries by RowId. Moroover, the list of RowEntries is always smaller
   * than the worst case, in which the block is filled with row pieces of size 1, so there will be at max
   * blocksize-sizeof(Data::BlockDef) entries. For a 4KiB blccok that is.
   *
   * IndexLeaf key entries point to rowids in this block thorugh (BlockId,RowId) pairs.
   * Decoupling the RowId from its index location in the RowEntry list allows to move data around within the block
   * without consequence to other use (locking takes place where required) - and hence avoid fragmented free space.
   *
   * Data is allocated from the end of the block downwards.
   *
   * If a row is too big for one block, it is continued in other blocks, in which case
   * RowEntry::next_block and RowEntry::next_rowid point to the next block, which itself may continue. If the row fits within
   * the block, RowEntry::next_block will be zero.
   *
   * It is possible to
   */
  class Data : GenericBlock {
    public:

      /**
       * Tracks a row piece in a block.
       * 4 + 2 + 2 + 8 + 4 = 20
       */
      #pragma pack(1)
      struct RowEntry {
        /** The rowid, which may not be the same as the row's index in the RowEntry list. */
        RowId rowid;
        /** The offset, from this block start, where the row data is stored. */
        BlockSize offset;
        /** The size of this row data. */
        BlockSize size;
        /** If not 0, the block where the row continues. */
        BlockId next_block;
        /** If not 0, the row slot in next_block where the ro continues. */
        RowId next_rowid;
      };
      #pragma pack()

      /**
       * The block definition.
       * first_entry is the first entry in a list of num_rows RowEntry.
       */
      #pragma pack(1)
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The number of row entries in this block */
        RowId num_rows;
        /** The offset within the block where data resides. */
        BlockSize low_data_offset;
        /** The first row entry. */
        RowEntry first_entry;
      };
      #pragma pack()

      /**
       * Construct against an existing address.
       * @param store The KVStore the block belongs to.
       * @param address The address to interpret as a FileHeader.
       */
      Data( KVStore* store, void* address ) : GenericBlock( store ) { block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Initialize the block
       * @param id The BockId to initialize with.
       */
      void init( BlockId id );

      /**
       * Deep analysis, return true when all is well.
       * @param os The ostream to write analysis to.
       * @return True if the block is ok.
       */
      virtual bool analyze( std::ostream &os );

      /**
       * Provide access to the block.
       * @return A reference to the FileHeader::BlockDef
       */
      BlockDef& getBlock() { return *block_; }

      /**
       * Get the number of free bytes in the block - a value of the returned size would fit. Note that this number is sizeof(RowEntry)
       * less than the number of unused bytes in the block.
       * @return The free space in bytes.
       */
      BlockSize getFreeSpace() const;

      /**
       * Allocate a row with the given size and return the RowId within this block. It is the caller's responsability
       * to not overallocate (size <=  getFreeSpace()).
       * @param size The size in bytes to allocate.
       * @return The RowId of the allocated row.
       */
      RowId allocateRow( BlockSize size );

      /**
       * Allocate a row matching the size of the OctetArray, and copy its contents.
       * @param src The source OctetArray.
       * @return The RowId of the allocated row.
       */
      RowId allocateRow( const common::OctetArray& src );

      bool getRowData( RowId rowid, common::OctetArray& data ) const;

      /**
       * Release or de-allocate the rowid. This call moves remaining data if gaps in the free space would be created.
       * @param rowid The RowId to release.
       */
      void releaseRow( RowId rowid );

      /**
       * Set the next row where the row with rowid continues.
       * @param rowid The row to chain.
       * @param next_blockid The BlockId where the row continues.
       * @param next_rowid The RowId within next_blockid where the row continues.
       */
      void setNextRow( RowId rowid, BlockId next_blockid, RowId next_rowid );

    protected:

      /**
       * Pair of RowEntry and its index.
       */
      struct RowEntryIndexPair {
        /** RowEntry* of the pair. */
        RowEntry* entry;
        /** Index of the pair. */
        RowId     index;
      };

      /**
       * Return the index where a new rowid needs to be inserted to maintain ordering in the RowEntry list. This
       * function performs a binary search and is calling itself recursively.
       * @param rowid The new rowid to be inserted.
       * @param low The low index for the search.
       * @param high The high index for the search.
       * @return The index at which to insert.
       * \note Time complexity O(log(block_>num_rows))
       */
      RowId insertSearch( RowId rowid, RowId low, RowId high ) const;

      /**
       * Return the index in the RowEntry list for the rowid. Returns InvalidRowId when the rowid is not in the list..
       * @param rowid The rowid to search for.
       * @param low The low index for the search.
       * @param high The high index for the search.
       * @return The index of the rowid or InvalidRowId if the rowid is not found.
       * \note Time complexity O(log(block_>num_rows))
       */
      RowId matchSearch( RowId rowid, RowId low, RowId high ) const;

      /**
       * Return the RowEntryIndexPair for the rowid.
       * @param rowid The rowid to search for.
       * @return The RowEntryIndexPair for the matched rowid.
       */
      RowEntryIndexPair matchSearch( RowId rowid ) const {
        if ( block_->num_rows > 0 ) {
          RowId index = matchSearch( rowid, 0, block_->num_rows - 1 );
          return { getRowEntryByIndex(index), index };
        } else return { nullptr, InvalidRowId };
      }

      /**
       * Find a free RowId - the lowest unused rowid.
       * @param low The low index for the search.
       * @param high The high index for the search.
       * @return A currently unused RowId.
       * \note Time complexity O(log(block_->num_rows))
       */
      RowId findFreeRowId( RowId low, RowId high ) const;

      /**
       * Get a free RowId. As previous existing rowids might get deleted this function returns
       * the lowest unused rowid.
       * @return A currently unused RowId.
       */
      RowId getFreeRowId() const;

      /**
       * Return a pointer to the first RoWentry.
       * @return A pointer the the first RowEntry.
       */
      Data::RowEntry* getFirstRowEntry() const;

      /**
       * Return a const pointer to the first RoWentry.
       * @return A const pointer the the first RowEntry.
       */
      const Data::RowEntry* getFirstConstRowEntry() const;

      /**
       * Get a pointer to a RowEntry by index.
       * @param index The index of the RowEntry to return.
       * @return A pointer to a RowEntry by index.
       * \note Time complexity O(1)
       */
      Data::RowEntry* getRowEntryByIndex( RowId index ) const;

      /**
       * Get a pointer to a RowEntry by RowId. Returns nullptr when the rowid is not found.
       * @param rowid The rowid of the RowEntry to return.
       * @return A pointer to a RowEntry by index, or a nullptr if not found.
       * \note Time complexity O(log(block_->num_rows))
       */
      Data::RowEntry* getRowEntryByRowId( RowId rowid ) const;

      /**
       * Return a byte pointer to the address + offset bytes.
       * @param offset The offset relative to block_.
       * @return a uint8_t pointer to the offset within the block.
       */
      uint8_t* bytePtr( BlockSize offset ) const { return reinterpret_cast<uint8_t*>( block_ ) + offset; }

      /**
       * Return a void pointer to the address + offset bytes.
       * @param offset The offset relative to block_.
       * @return a voud pointer to the offset within the block.
       */
      void* voidPtr( BlockSize offset ) const { return reinterpret_cast<void*>( bytePtr( offset ) ); }

      /**
       * The interpreted block.
       */
      BlockDef* block_;

  };

}

#endif