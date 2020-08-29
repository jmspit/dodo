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
 * @file kvstore.hpp
 * Defines the dodo::store::KVStore class.
 */

#ifndef store_kvstore_blockdefs_toc_hpp
#define store_kvstore_blockdefs_toc_hpp

#include <string>

#include <store/kvstore/blockdefs/toc.hpp>

namespace dodo::store::kvstore {

  /**
   * Table of Contents block, tracks index blocks and free blocks.
   */
  class TOC {
    public:

      /**
       * The block definition.
       */
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The next TOC blockid or 0 if this is the last TOC. */
        BlockId next_toc;
        /** The number of entries in this TOC. */
        uint64_t entries;
        /**
         * The BlockType of the first of a list of entries.
         * The list is ordered, the first entry points to BlockId 2, as BlockId 0 is the FileHeader
         * and BlockId 1 is the (first) TOC.
         */
        BlockType entry;
      };

      /**
       * Construct against an existing address.
       * @param address The address to interpret as a FileHeader.
       */
      TOC( void* address ) { block_ = reinterpret_cast<BlockDef*>( address ); }



    protected:
      /**
       * The interpreted block.
       */
      BlockDef* block_;
  };

}

#endif