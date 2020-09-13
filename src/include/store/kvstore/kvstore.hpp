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
#include <common/systemerror.hpp>

#include <store/kvstore/blockdefs/common.hpp>
#include <store/kvstore/blockdefs/data.hpp>
#include <store/kvstore/blockdefs/indexleaf.hpp>
#include <store/kvstore/blockdefs/indextree.hpp>
#include <store/kvstore/blockdefs/toc.hpp>

namespace dodo::store::kvstore {

  using namespace std;
  using namespace common;

  /**
   * A Key-value store.
   *
   *   - Keys are strings.
   *   - Values are OctetArray instances.
   *   - A Key has a mime type, so the OctetArray could contain mime type "application/json"
   *   - A key can have attributes 'compressed' and 'encrypted', and values would be transparently
   *     compressed/decompressed or encrypted/decrypted on the fly.
   *
   * The store is block-oriented with a blocksize that must be an integer multiple of the system page size
   * (typically 4KiB). The block size has implications for the maximum key length, which is slightly less than
   * half the blocksize.
   *
   * The store is thread-safe, share a single KVStore object, which maps to a single file, among multiple threads.
   * Synchronization (locking) is done a block level:
   *
   *   - Readers are not blocked by readers even on the same block.
   *   - Readers are blocked by writers on the same block.
   *   - Writers are blocked by readers on the same block.
   *   - Writers are blocked by writers on the same block.
   *
   * Operations aqcuire and release read and write locks as quickly as possible. Operations that need to lock more
   * than one block always lock in order, from low blockid to high. For example, to get a key value pair, the KVStore
   * cannot first find the key, release the read lock on the index block, only to discover the row has disapperead from the
   * data block in the meantime, it mst hold aread lock on both the index and data bpock until the value pair is copied into
   * a caller-side result.
   *
   * Some blocks are used more heavily than others, such as TOC and IndexTree blocks.
   *
   * The store provides basic operations:
   *
   *   - create a ney key-value pair.
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
       * Data type for the key of  akey-value pair.
       */
      typedef std::string Key;

      /**
       * The share mode of the KVStore.
       */
      enum class ShareMode {
        Private,        /**< Do not share outside process (but sharing between threads possible). */
        Shared,         /**< Share with other processes. */
        Clustered,      /**< Share with other processes on other nodes (which requires a cluster filesystem). */
      };

      KVStore();

      virtual ~KVStore();

      /**
       * Open the KVStore. The file must exist and be valid (correct header and structure).
       * @param path The KVStore file path.
       * @param mode The ShareMode to apply.
       * @return The SystemError matching errno returned by open (man 2 open) or SystemError::ecOK.
       */
      SystemError open( const std::filesystem::path &path, ShareMode mode );

      /**
       * Init the KVStore. The file must not exist.
       * @param path The KVStore file path.
       * @param blocksize The KVStore block size, ceiled to multiples of 4KiB.
       * @param blocks The number of blocks.
       * @param mode The ShareMode to apply.
       * @return The SystemError matching errno returned by open (man 2 open) or SystemError::ecOK.
       */
      SystemError init( const std::filesystem::path &path, BlockSize blocksize, BlockId blocks, ShareMode mode );

      /**
       * Add blocks to the file.
       * @param blocks The number of blocks to add.
       */
      void extend( BlockId blocks );

      /**
       * Analyze the store and produce fiudings to os.
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
       * Return the filedescriptor fd_.
       * @return The file descriptor.
       */
      int getFD() const { return fd_; }

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
       * typedef the lock functions so the lock functions can be initialized depending on the ShareMode
       * and avoid ShareMode detection within the lock functions.
       */
      typedef void (KVStore::*waitLockFN)( BlockId blockid );


      /**
       * The read lock function in use.
       */
      waitLockFN waitReadLock_;

      /**
       * The read unlock function in use.
       */
      waitLockFN waitReadUnlock_;

      /**
       * The Write lock function in use.
       */
      waitLockFN waitWriteLock_;

      /**
       * The Write unlock function in use.
       */
      waitLockFN waitWriteUnlock_;

      /**
       * Wait for and lock the blockid for reading in Private mode.
       * As in private mode there is no need to sync access between clusters or processes, this
       * only uses a Mutex to sync.
       * @param blockid The blockid to lock.
       */
      void waitReadLockPrivate( BlockId blockid ) {};

      /**
       * Wait for and lock the blockid for writing in Private mode.
       * As in private mode there is no need to sync access between clusters or processes, this
       * only uses a Mutex to sync.
       * @param blockid The blockid to lock.
       */
      void waitWriteLockPrivate( BlockId blockid ) {};

      /**
       * Wait for and unlock the blockid for reading in Private mode.
       * As in private mode there is no need to sync access between clusters or processes, this
       * only uses a Mutex to sync.
       * @param blockid The blockid to unlock.
       */
      void waitReadUnlockPrivate( BlockId blockid ) {};

      /**
       * Wait for and unlock the blockid for writing in Private mode.
       * As in private mode there is no need to sync access between clusters or processes, this
       * only uses a Mutex to sync.
       * @param blockid The blockid to unlock.
       */
      void waitWriteUnlockPrivate( BlockId blockid ) {};

      /**
       * Wait for and lock the blockid for reading in Shared mode.
       * The file and memory may be shared between processes.
       * @param blockid The blockid to lock.
       */
      void waitReadLockShared( BlockId blockid ) {};

      /**
       * Wait for and lock the blockid for writing in Shared mode.
       * As in private mode there is no need to sync access between clusters or processes, this
       * only uses a Mutex to sync.
       * @param blockid The blockid to lock.
       */
      void waitWriteLockShared( BlockId blockid ) {};

      /**
       * Wait for and unlock the blockid for reading in Shared mode.
       * The file and memory may be shared between processes.
       * @param blockid The blockid to unlock.
       */
      void waitReadUnlockShared( BlockId blockid ) {};

      /**
       * Wait for and unlock the blockid for writing in Shared mode.
       * As in private mode there is no need to sync access between clusters or processes, this
       * only uses a Mutex to sync.
       * @param blockid The blockid to unlock.
       */
      void waitWriteUnlockShared( BlockId blockid ) {};

      /** The block size. */
      BlockSize blocksize_;

      /** The minimum amount of blocks to allocate - kvstore file cannot be smaller. */
      const BlockId min_blocks_ = 4;

      /** The file descriptor. */
      int fd_;

      /** The mapped memory address. */
      void *address_;


  };

}

#endif