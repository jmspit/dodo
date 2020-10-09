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
 * @file indextree.hpp
 * Defines the dodo::store::kvstore::IndexTree class.
 */

#ifndef store_kvstore_blockdefs_index_tree_hpp
#define store_kvstore_blockdefs_index_tree_hpp

#include <store/kvstore/blockdefs/common.hpp>

namespace dodo::store::kvstore {

  /**
   * An index tree block. A tree block only contains key entries that point to either other IndexTree blocks
   * or to IndexLeaf blocks - so an IndexTree block always has children.
   */
  class IndexTree {
    public:

      /**
       * An index tree entry. Each entry stores a key and the blockid to the left of that key.
       */
      #pragma pack(1)
      struct IndexTreeEntry {
        /** the offset from block start where the key is stored. */
        BlockSize offset;
        /** the size of the key. */
        BlockSize size;
        /** The BlockId the entry points to. */
        BlockId blockid;
      };
      #pragma pack()

      /**
       * The block definition.
       */
      #pragma pack(1)
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The number of index entries in the block. */
        RowId num_entries;
        /** The rightmost child - the right-child of the last key on the block, or 0 if there are no keys in this block. */
        BlockId rightmost;
        /** The first of a list of indexentries in the block. */
        IndexTreeEntry first;
      };
      #pragma pack(1)

      /**
       * Construct against an existing address.
       * @param blocksize The blocksize of the block.
       * @param address The address to interpret as a FileHeader.
       */
      IndexTree( BlockSize blocksize, void* address ) { blocksize_ = blocksize; block_ = reinterpret_cast<BlockDef*>( address ); }

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