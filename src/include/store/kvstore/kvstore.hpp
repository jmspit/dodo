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

#ifndef store_kvstore_kvstore_hpp
#define store_kvstore_kvstore_hpp

#include <filesystem>
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include <common/systemerror.hpp>

#include <store/kvstore/blockdefs/common.hpp>
#include <store/kvstore/blockdefs/data.hpp>
#include <store/kvstore/blockdefs/indexleaf.hpp>
#include <store/kvstore/blockdefs/indextree.hpp>
#include <store/kvstore/blockdefs/toc.hpp>
#include <store/kvstore/blocklock.hpp>

namespace dodo::store::kvstore {

  using namespace std;
  using namespace common;

  class ReadBlockLock {
  public:

    /**
     * Locks the BlockId in read / share mode.
     */
    ReadBlockLock( BlockId blockid );

    /**
     * Unlocks the BlockId from read / share mode.
     */
    ~ReadBlockLock();

  protected:
    struct flock lock_;
    int fd_;
};

class WriteBlockLock {
  public:

    /**
     * Locks the BlockId in write / unique mode.
     */
    WriteBlockLock( BlockId blockid );

    /**
     * Unlocks the BlockId from write / unique mode.
     */
    ~WriteBlockLock();

  protected:
    struct flock lock_;
    int fd_;
};

  /**
   * A Key-value store.
   *
   *   - Keys are strings.
   *   - Values are OctetArray instances.
   *   - A Key has a mime type, so the OctetArray could contain mime type "application/json"
   *   - A key can have attributes 'compressed' and 'encrypted', and values would be transparently
   *     compressed/decompressed or encrypted/decrypted on the fly.
   *
   * The store is block-oriented with a block-size that must be an integer multiple of the system page size
   * (typically 4KiB). The block size has implications for the maximum key length, which is slightly less than
   * half the block-size.
   *
   * The store is thread-safe, threads within a process share a single KVStore object, which maps to a single file,
   * and synchronization is handled by the KVSTore instance. The locking mechanism is shared-exclusive, writers acquire
   * exclusive locks, readers acquire shared locks. A shared lock blocks only exclusive locks, an exclusive lock blocks
   * both shared and exclusive locks.
   *
   *   - Readers are not blocked by readers even on the same block.
   *   - Readers are blocked by writers on the same block.
   *   - Writers are blocked by readers on the same block.
   *   - Writers are blocked by writers on the same block.
   *
   * Operations acquire and release read and write locks as quickly as possible. Operations that need to lock more
   * than one block always lock in order, from low block-id to high. For example, to get a key value pair, the KVStore
   * cannot first find the key, release the read lock on the index block, only to discover the row has disappeared from the
   * data block in the meantime, it must hold a read lock on both the index and data block until the value pair is copied into
   * a caller-side result.
   *
   * Some blocks are used more heavily than others, such as TOC and IndexTree blocks.
   *
   * The store provides basic operations:
   *
   *   - create a new key-value pair.
   *   - overwrite the value of a key.
   *   - delete a key.
   *   - get a value by key.
   *   - check for key existence.
   *
   * And additionally,
   *
   *   - get a list of values by key pattern.
   *   - export to json.
   *   - import from json.
   *
   */
  class KVStore {

    public:

      /**
       * Data type for the key of a key-value pair.
       */
      typedef std::string Key;

      /**
       * The allocation policy (assign data to blocks) for new rows.
       */
      enum class AllocationPolicy {
        EmptiestBlock,
        FullestBlock,
      };

      /**
       * Group configuration parameters.
       */
      struct Configuration {
        /**
         * The minimum number of blocks that will be added to the file size when a file resize is required.
         */
        BlockCount extend_size = 16;

        /**
         * BlockIds are assigned a map by the modulo (block_mutex_modulo) of the block id.
         * @see store::kvstore::BlockLock
         */
        BlockId block_mutex_modulo = 32;

        /**
         * If free space in a block would fall below max_free * free_ratio, new allocations are made in
         * another block that meets this criterion (blocks are allocated if need be).  A higher value will
         * prevent data to move between blocks when an update increases the size, which requires a write
         * lock on multiple key index blocks.
         */
        double free_ratio;
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
       * Initialize the KVStore. The file must not exist.
       * @param path The KVStore file path.
       * @param blocksize The KVStore block size, rounded upwards to multiples of 4KiB.
       * @param blocks The number of blocks.
       * @return The SystemError matching errno returned by open (man 2 open) or SystemError::ecOK.
       */
      SystemError init( const std::filesystem::path &path, BlockSize blocksize, BlockId blocks );

      /**
       * Attach the calling thread to the KVStore so that access is synchronized.
       * Threads that seek to use the KVStore should call attachThread before calling other KVStore members,
       * and call releaseThread when the thread will no longer access the KVStore.
       * @return the file descriptor
       */
      int attachThread();

      /**
       * Notify the KVStore that the calling thread will no longer need to be synchronized / will not
       * call KVStore members anymore.
       */
      void releaseThread();

      /**
       * Add blocks to the file after acquiring a unique lock on the big_mutex_.
       * @param blocks The number of blocks to add.
       */
      void extend( BlockId blocks );

      /**
       * Analyze the store and produce findings to the OS.
       * @param os The stream to write the analysis to.
       * @return true if the KVStore (file) is valid.
       */
      bool analyze( std::ostream &os );

      /**
       * Return the BlockSize of the KVStore.
       * @return The BlockSize.
       */
      BlockSize getBlockSize() const { return blocksize_; }

      /**
       * Return the file descriptor for the calling thread.
       * @return The file descriptor for the calling thread.
       */
      int getFD();

      static KVStore* getKVStore();

    protected:

      /**
       * Return a pointer to the block's BlockHeader.
       * @param blockid the BlockId to get the address for.
       * @return a pointer to the block's BlockHeader.
       */
      BlockHeader* getBlockAddress( BlockId blockid ) const {
        return reinterpret_cast<BlockHeader*>( (reinterpret_cast<char*>(address_) + blocksize_ * blockid) );
      }

      /**
       * Return the file descriptor for the calling thread.
       */
      int getThisThreadFD();

      /** The block size. */
      BlockSize blocksize_;

      /** The minimum amount of blocks to allocate - kvstore file cannot be smaller. */
      const BlockId min_blocks_ = 4;

      /** The mapped memory address. Note that the memory region may be relocated by a call to extend. */
      void *address_;

      /**
       * As we are using OFD advisory locking (man fcntl), each thread will need its own
       * file descriptor.
       */
      std::map<std::thread::id,int> fd_map_;

      /**
       * Mutex to protect (only) the fd_map_;.
       */
      std::shared_timed_mutex fd_map_mutex_;

      /**
       * Locks initialization and maintenance. Other methods lock this shared. The lock
       * guards relocation of the mapped memory which (may) cause the address_ pointer
       * to change. More or less halts all concurrent operation against the KVStore.
       */
      std::mutex big_mutex_;

      /**
       * The path to the memory mapped kvstore file.
       */
      std::filesystem::path path_;

      static KVStore *singleton_;


  };

}



#endif