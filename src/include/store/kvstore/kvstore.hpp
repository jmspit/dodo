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

#ifndef store_kvstore_hpp
#define store_kvstore_hpp

#include <filesystem>
#include <unistd.h>
#include <common/systemerror.hpp>

namespace dodo::store {

  using namespace std;
  using namespace common;

  /**
   * Key-value store backed by memory mapped file.
   */
  class KVStore {

    public:

      /**
       * Data type for the key of  akey-value pair.
       */
      typedef std::string Key;

      /**
       * The share mode of the KVStore.
       */
      enum class ShareMode {
        Private,        /**< Do not share outside process (sharing between threads allowed). */
        Shared,         /**< Share with other processes. */
      };

      KVStore();

      virtual ~KVStore();

      /**
       * Open the KVStore. The file must exist and be valid (correct header and structure).
       * @param path The KVStore file path.
       * @return The SystemError matching errno returned by open (man 2 open) or SystemError::ecOK.
       */
      SystemError open( const std::filesystem::path &path );

      /**
       * Init the KVStore. The file must not exist.
       * @param path The KVStore file path.
       * @param size The KVStore size (rounded up to 4KiB blocks).
       * @return The SystemError matching errno returned by open (man 2 open) or SystemError::ecOK.
       */
      SystemError init( const std::filesystem::path &path, ssize_t size );

      void analyze( std::ostream &os );

    protected:

      /**
       * Data type for block ids
       */
      typedef uint64_t BlockId;

      /**
       * Block type enumerator.
       */
      enum BlockType: uint32_t {
        Free = 0,         /**< Free block. */
        FileHeader = 1,   /**< File header block */
        TOC = 2,          /**< TOC block. */
        Index = 3,        /**< Index block */
        Data = 4          /**< Data block */
      };

      /**
       * KVStore block header.
       */
      struct BlockHeader {
        /** The BlockId of the block */
        BlockId blockid;
        /** The type of block **/
        BlockType blocktype;
        /** The CRC32 checksum on the remainder of the block. */
        uint32_t crc32;
      };

      /**
       * KVStore file header.
       */
      struct Header {
        /** The block header. */
        BlockHeader block_header;
        /** The header magic. */
        uint64_t magic;
        /** The file size in blocks */
        ssize_t blocks;
        /** The file version. */
        uint16_t version;
        /** The size of the name field */
        uint16_t name_sz;
        /** The size of the description field */
        uint16_t description_sz;
        /** The size of the contact field */
        uint16_t contact_sz;
        // name, description and contact follow
      };

      /**
       * KVStore TOC block header. The TOC, which may comprise multiple blocks, tracks free and index block locations.
       */
      struct TOCHeader {
        /** The block header. */
        BlockHeader block_header;
        /** The next TOC or 0 if this is the last TOC block. */
        BlockId next_toc;
        /** The index list starts after the blockheader upwards. */
        uint16_t index_entries;
        /** The free list starts at the end of the page downwards. */
        uint16_t freelist_entries;
      };

      /**
       * KVStore Index block header.
       */
      struct IndexHeader {
        /** The block header. */
        BlockHeader block_header;
      };

      /**
       * Return a pointer to the block's BlockHeader.
       * @param blockid the BlockId to get the address for.
       * @return a pointer to the block's BlockHeader.
       */
      BlockHeader* getBlockAddress( BlockId blockid ) const {
        return reinterpret_cast<BlockHeader*>( (reinterpret_cast<char*>(address_) + pagesize_ * blockid) );
      }

      /**
       * The version of this KVStore code, facilitating auto upgrades. Format is [major][minor][patch]L where each
       * field is a decimal ranging from 0-99, so 1.2.13 would be 010213L.*/
      const uint16_t version = 000001;

      /** The file header magic. */
      const uint64_t magic = 2004196816041969;

      /** The page size. */
      size_t pagesize_ = 4096;

      /** The minimum amount of blocks to allocate - kvstore file cannot be smaller. */
      const BlockId min_blocks_ = 4;

      /** The file descriptor. */
      int fd_;

      /** The file size. */
      ssize_t size_;

      /** The file block count (redundant field as it can be derived from size_ and pagesize_, to avoid repeated arithmetic) */
      BlockId blocks_;

      /** The mapped memory address. */
      void *address_;


  };

}

#endif