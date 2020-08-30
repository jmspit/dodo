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

#ifndef store_kvstore_blockdefs_data_hpp
#define store_kvstore_blockdefs_data_hpp

#include <store/kvstore/blockdefs/common.hpp>

namespace dodo::store::kvstore {

  /**
   * A data block.
   */
  class Data {
    public:

      /**
       * Tracks a row piece in a block.
       */
      struct RowEntry {
        /** If not 0, the block where the row continues. */
        uint64_t next_block;
        /** If not 0, the row slot in next_block where the ro continues. */
        uint16_t next_row;
        /** The offset, from the start of this bock, to where the row data starts. */
        uint16_t offset;
        /** The size of the row. */
        uint16_t size;
      };

      /**
       * Tracks free space in a block.
       */
      struct FreeEntry {
        /** The offset, from the start of this bock, to where the row data starts. */
        uint16_t offset;
        /** The size of the row. */
        uint16_t size;
      };      

      /**
       * The block definition.
       * first_entry is the first entry in a list of num_rows RowEntry.
       * For the end of the block downwards, there are num_free FreeEntry
       * structures.
       */
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The number of row entries in this block */
        uint16_t num_rows;
        /** The number of row entries in this block */
        uint16_t num_free;
        /** The first row entry. */
        RowEntry first_entry;
      };

      /**
       * Construct against an existing address.
       * @param blocksize The blocksize of the block.
       * @param address The address to interpret as a FileHeader.
       */
      Data( uint64_t blocksize, void* address ) { blocksize_ = blocksize; block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Initialize the block
       * @param id The BockId to initialize with.
       */
      void init( BlockId id );

    protected:
      /** The minimum number of FreeEntry slots. */
      const uint16_t num_free_min = 10;

      /** The block size. */
      uint64_t blocksize_;

      /**
       * The interpreted block.
       */
      BlockDef* block_;   

  };

}

#endif