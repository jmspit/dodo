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
 * @file common.hpp
 * Defines dodo::store::kvstore commpon things.
 */

#ifndef store_kvstore_blockdefs_common_hpp
#define store_kvstore_blockdefs_common_hpp

#include <Crc32.h>

namespace dodo::store::kvstore {

  /**
   * Data type for block ids
   */
  typedef uint64_t BlockId;

  /**
   * Block type enumerator.
   */
  enum BlockType: uint32_t {
    btFree = 0,         /**< Free block. */
    btFileHeader = 1,   /**< File header block */
    btTOC = 2,          /**< TOC block. */
    btIndex = 3,        /**< Index block */
    btData = 4          /**< Data block */
  };

  /**
   * KVStore block header.
   */
  struct BlockHeader {
    /** The BlockId of the block */
    BlockId blockid;
    /** The type of block **/
    BlockType blocktype;
    /** The CRC32 checksum on the contents of the block (excluding the crc32 itself). */
    uint32_t crc32;

    /**
     * Calculate the crc32 of the block (excluding the crc32 itself).
     * @param blocksize The size of the block in bytes.
     * @return The crc32 on the data.
     */
    uint32_t calcCRC32( size_t blocksize );

    /**
     * Calculate the crc32 of the block and set it in the crc32 field.
     * @param blocksize The size of the block in bytes.
     */
    void syncCRC32( size_t blocksize );

    /**
     * zero he block, which is assumed to be blocksize bytes.
     * @param blocksize The size of the block in bytes.
     */
    void zero( size_t blocksize );

    /**
     * Initialize the block header.
     * @param blocksize The blocksize of the block.
     * @param id The BlockId of the block.
     * @param type The BlockType of the block.
     */
    void init( uint64_t blocksize, BlockId id, BlockType type ) {
      this->zero( blocksize );
      this->blockid = id;
      this->blocktype = type; 
    }    

  };


}

#endif