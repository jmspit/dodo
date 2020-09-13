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

#include <iostream>
#include <functional>

namespace dodo::store::kvstore {

  /**
   * Data type for block ids
   *
   * 8 bytes.
   */
  typedef uint64_t BlockId;

  /**
   * Data type for row id's. As a row could contain as little as one byte,
   * this effectively limits the size of a block. If this was an uint16_t for example,
   * there could only be roughly 65536 1 byte rows in a block, limiting the blocksize to 64k.
   * To avoid that limit and allow maximum blocksize of slightly less than 4G, which is an insane value -
   * best use a blocksize <= 1MiB at most.
   *
   * 4 bytes.
   */
  typedef uint32_t RowId;

  /**
   * RowId value to be interpreted as invalid.
   */
  const RowId InvalidRowId = 0xFFFFFFFF;

  /**
   * Data type for block sizes.
   */
  typedef uint32_t BlockSize;

  /**
   * Data type for block counts (the number of blocks in a file), which must accomodate at least BlockId
   * unique ids.
   */
  typedef BlockId BlockCount;

  /**
   * Data type for file sizes.
   */
  typedef uint64_t FileSize;

  /**
   * Block type enumerator.
   *
   * 4 bytes.
   */
  enum BlockType: uint32_t {
    btFree = 0,         /**< Free block. */
    btFileHeader = 1,   /**< File header block */
    btTOC = 2,          /**< TOC block. */
    btIndexTree = 3,    /**< Index tree block */
    btIndexLeaf = 4,    /**< Index leaf block */
    btData = 5          /**< Data block */
  };

  /**
   * KVStore block header.
   * 16 bytes.
   */
  #pragma pack(1)
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
    uint32_t calcCRC32( BlockSize blocksize ) const;

    /**
     * Calculate the crc32 of the block and set it in the crc32 field.
     * @param blocksize The size of the block in bytes.
     */
    void syncCRC32( BlockSize blocksize );

    /**
     * Verify the crc32 of the block by calculating the crc32 and comparing it to the crc32 field stored
     * in the block.
     * @param blocksize The size of the block in bytes.
     * @return True if the crc32 stored in the block is the same as the crc32 calculated over the block.
     */
    bool verifyCRC32( BlockSize blocksize ) const;

    /**
     * zero he block, which is assumed to be blocksize bytes.
     * @param blocksize The size of the block in bytes.
     */
    void zero( BlockSize blocksize );

    /**
     * Initialize the block header.
     * @param blocksize The blocksize of the block.
     * @param id The BlockId of the block.
     * @param type The BlockType of the block.
     */
    void init( BlockSize blocksize, BlockId id, BlockType type ) {
      this->zero( blocksize );
      this->blockid = id;
      this->blocktype = type;
    }
    #pragma pack()

  };

  class KVStore;

  /**
   * Common block ancestor.
   */
  class GenericBlock {
    public:

      /**
       * Create against a KVStore.
       * @param store The KVStore this block belongs to.
       */
      GenericBlock( KVStore* store ) : store_(store) {};

      /** Destructor. */
      virtual ~GenericBlock() {};

      /**
       * Analyze the block and report findings (not only errors) in os.
       * @param os The stream to write findings to.
       * @return True when the block is ok.
       */
      virtual bool analyze( std::ostream &os ) = 0;

    protected:
      /**
       * Write a header with caption to the stream.
       * @param os The stream to write to.
       * @param caption The caption to write.
       */
      void outputHeader( std::ostream& os, const std::string &caption );

      /**
       * The KVStore for which this object was created.
       */
      KVStore* store_;
  };

  /**
   * Tester code convenience class.
   */
  class Tester {
    public:
      /**
       * Construct
       * @param os Stream to write test results to
       */
      Tester( std::ostream &os ) :os_(os), ok_(true) {}

      /** Destruct */
      virtual ~Tester() {}

      /**
       * Test
       * @param msg A descriptive message
       * @param test A callable (function, lambda) delivering the test result.
       * @return The test result AND the result of the previous test.
       */
      bool test( const std::string &msg,
                 std::function<bool(void)> test ) {
        if ( !ok_ ) return ok_;
        ok_ = ok_ && std::invoke( test );
        if ( ok_ ) os_ << "OK   "; else os_ << "FAIL ";
        os_ << msg << std::endl;
        return ok_;
      }

    private:
      /** Output stream */
      std::ostream &os_;
      /** Test result. */
      bool ok_;
  };


}

#endif