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
 * @file indexleaf.hpp
 * Defines the dodo::store::kvstore::IndexLeaf class.
 */

#ifndef store_kvstore_blockdefs_index_leaf_hpp
#define store_kvstore_blockdefs_index_leaf_hpp

#include <store/kvstore/blockdefs/common.hpp>

namespace dodo::store::kvstore {

  /**
   * An index leaf block.
   */
  class IndexLeaf {
    public:

      /**
       * An index entry.
       */
      #pragma pack(1)
      struct IndexEntry {
        /** the offset from block start where the key is stored. */
        BlockSize offset;
        /** the size of the key. */
        BlockSize size;
        /** The BlockId the entry points to. */
        BlockId block;
        /** The row the entry points to. */
        RowId row;
      };
      #pragma pack()

      /**
       * The block definition.
       */
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
      };

      /**
       * Construct against an existing address.
       * @param blocksize The blocksize of the block.
       * @param address The address to interpret as a FileHeader.
       */
      IndexLeaf( BlockSize blocksize, void* address ) { blocksize_ = blocksize; block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Initialize the block
       * @param id The BockId to initialize with.
       */
      void init( BlockId id );

      /**
       * Provide access to the block.
       * @return A reference to the FileHeader::BlockDef
       */
      BlockDef& getBlock() { return *block_; }

    protected:

      /** The block size. */
      BlockSize blocksize_;

      /**
       * The interpreted block.
       */
      BlockDef* block_;

  };

}

#endif