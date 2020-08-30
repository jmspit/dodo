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

#include <store/kvstore/blockdefs/header.hpp>
#include <store/kvstore/blockdefs/toc.hpp>

namespace dodo::store::kvstore {

  /**
   * Table of Contents block, tracks BlockId to BlockType
   */
  class TOC {
    public:

      /**
       * The block definition.
       */
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The lowest BlockId tracked in this block, only valid when entries > 0. */
        BlockId lowest;
        /** The highest BlockId tracked in this block, only valid when entries > 0. */
        BlockId highest;        
        /** The next TOC blockid or 0 if this is the last TOC. */
        BlockId next_toc;
        /** The number of entries in this TOC. */
        uint64_t entries;
        /**
         * The BlockType of the first of a list of entries.
         */
        BlockType entry;

      };

      /**
       * Construct against an existing address.
       * @param blocksize The size of the block
       * @param address The address to interpret as a FileHeader.
       */
      TOC( uint64_t blocksize, void* address ) { blocksize_ = blocksize; block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Provide access to the block.
       * @return A reference to the TOC::BlockDef
       */
      BlockDef& getBlock() { return *block_; }

      /**
       * Set the next TOC.
       * @param id The BlockId of the next TOC.
       */
      void setNextTOC( BlockId id ) { block_->next_toc = id; }

      /**
       * Return true when id is >= lowest and <= highest
       * @param id The BlockId to match.
       * @return True when id >= lowest && <= highest
       */
      bool hasBlock( BlockId id ) const { return id >= block_->lowest && id <= block_->highest; };

      /**
       * Initialize the block
       * @param id The BockId to initialize with.
       */
      void init( BlockId id );

      /**
       * Set the entry of BlockId id to BlockType type.
       * @param id The BlockId
       * @param type The BlockType
       */
      void setEntry( BlockId id, BlockType type );

    protected:
      /** The block size. */
      uint64_t blocksize_;

      /**
       * Return the maximum number of entries in a TOC block.
       * @return The max number of entries in a TOC block.
       */
      uint64_t getMaxEntries() const { return (blocksize_ - sizeof( BlockDef ) + sizeof( BlockType ))/sizeof( BlockType );  }

      /** The interpreted block. */
      BlockDef* block_;
  };

}

#endif