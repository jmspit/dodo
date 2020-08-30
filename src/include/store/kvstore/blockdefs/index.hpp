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
 * @file index.hpp
 * Defines the dodo::store::kvstore::Index class.
 */

#ifndef store_kvstore_blockdefs_index_hpp
#define store_kvstore_blockdefs_index_hpp

#include <store/kvstore/blockdefs/common.hpp>

namespace dodo::store::kvstore {

  /**
   * An index block.
   */
  class Index {
    public:

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
      Index( uint64_t blocksize, void* address ) { blocksize_ = blocksize; block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Initialize the block
       * @param id The BockId to initialize with.
       */
      void init( BlockId id );
   
    protected:

      /** The block size. */
      uint64_t blocksize_;

      /**
       * The interpreted block.
       */
      BlockDef* block_;   

  };

}

#endif