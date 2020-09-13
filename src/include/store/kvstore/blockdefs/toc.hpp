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
 * @file toc.hpp
 * Defines the dodo::store::kvstore::TOC class.
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
  class TOC : GenericBlock {
    public:

      /**
       * The block definition.
       */
      #pragma pack(1)
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The lowest BlockId tracked in this block, only valid when entries > 0. */
        BlockId lowest;
        /** The highest BlockId tracked in this block, only valid when entries > 0. */
        BlockId highest;
        /** The next TOC blockid or 0 if this is the last TOC. */
        BlockId next_toc;
        /**
         * The BlockType of the first of a list of entries.
         */
        BlockType entry;

      };
      #pragma pack()

      /**
       * Construct against an existing address.
       * @param store The KVStore that owns this block.
       * @param address The address to interpret as a FileHeader.
       */
      TOC( KVStore* store, void* address ) : GenericBlock( store ) { block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Deep analysis, return true when all is well.
       * @param os The ostream to write analysis to.
       * @return True if the block is ok.
       */
      virtual bool analyze( std::ostream &os );

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
       * Return true when id is in this TOC.
       * @param id The BlockId to match.
       * @return True when id is in this TOC.
       */
      bool hasBlock( BlockId id ) const { return id >= block_->lowest && id <= block_->lowest + getMaxEntries(); };

      /**
       * Initialize the block
       * @param id The BlockId to initialize with.
       * @param lowest The lowest BlockId tracked in this TOC block.
       */
      void init( BlockId id, BlockId lowest );

      /**
       * Set the entry of BlockId id to BlockType type.
       * @param id The BlockId
       * @param type The BlockType
       */
      void setEntry( BlockId id, BlockType type );

    protected:
      /**
       * Return the maximum number of entries in a TOC block.
       * @return The max number of entries in a TOC block.
       */
      uint64_t getMaxEntries() const;

      /** The interpreted block. */
      BlockDef* block_;
  };

}

#endif