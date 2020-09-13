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
 * @file header.hpp
 * Defines the dodo::store::kvstore::FileHeader class.
 */

#ifndef store_kvstore_blockdefs_header_hpp
#define store_kvstore_blockdefs_header_hpp

#include <string>

#include <store/kvstore/blockdefs/common.hpp>

namespace dodo::store::kvstore {

  /**
   * The first block in the file.
   */
  class FileHeader : public GenericBlock {
    public:

      /**
       * The block definition.
       */
      #pragma pack(1)
      struct BlockDef {
        /** The block header. */
        BlockHeader block_header;
        /** The header magic. */
        uint64_t magic;
        /** The file blocksize in bytes - must be a multiple of sysconf(_SC_PAGESIZE) */
        BlockSize blocksize;
        /** The number of blocks in the file. */
        BlockCount blocks;
        /** The file version. */
        uint16_t version;
        /** when the file was first created. */
        int64_t created;
        /** The size of the name field */
        uint16_t name_sz;
        /** The size of the description field */
        uint16_t description_sz;
        /** The size of the contact field */
        uint16_t contact_sz;
        // name, description and contact follow
      };
      #pragma pack()

      /**
       * Construct against an existing address.
       * @param store The KVStore the block belongs to.
       * @param address The address to interpret as a FileHeader.
       */
      FileHeader( KVStore* store, void* address ) : GenericBlock( store ) { block_ = reinterpret_cast<BlockDef*>( address ); }

      /**
       * Provide access to the block.
       * @return A reference to the FileHeader::BlockDef
       */
      BlockDef& getBlock() { return *block_; }

      /**
       * Set the name, description and contact information.
       * @param name The name of the store.
       * @param description Brief description of the contents and purpose of the store.
       * @param contact The contact for issues with the store.
       */
      void setInfo( const std::string &name,
                    const std::string &description,
                    const std::string &contact );

      /**
       * Get the name, description and contact information.
       * @param name The name of the store.
       * @param description Brief description of the contents and purpose of the store.
       * @param contact The contact for issues with the store.
       */
      void getInfo( std::string &name,
                    std::string &description,
                    std::string &contact );

      /**
       * Initialize the FileHeader block.
       * @param blocks The number of blocks in the file.
       */
      void init( BlockCount blocks );

      /**
       * Deep analysis, return true when all is well.
       * @param os The ostream to write analysis to.
       * @return True if the block is ok.
       */
      virtual bool analyze( std::ostream &os );

      /**
       * The version of this KVStore code, facilitating auto upgrades. Format is [major][minor][patch]L where each
       * field is a decimal ranging from 0-99, so 1.2.13 would be 010213L.*/
      static const uint16_t version = 000001;

      /** The file header magic. */
      static const uint64_t magic = 2004196816041969;

    protected:

      /**
       * The interpreted block.
       */
      BlockDef* block_;
  };

}

#endif