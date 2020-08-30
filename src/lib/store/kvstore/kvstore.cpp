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
 * @file kvstore.cpp
 * Implements the dodo::store::KVStore class.
 */

#include <store/kvstore/kvstore.hpp>
#include <store/kvstore/blockdefs/header.hpp>
#include <store/kvstore/blockdefs/toc.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/mman.h>

#include <common/util.hpp>
#include <common/exception.hpp>


namespace dodo::store::kvstore {


  KVStore::KVStore() : fd_(0) {
    blocksize_ = sysconf(_SC_PAGESIZE);
  }

  KVStore::~KVStore() {
    if ( address_ ) {
      FileHeader fileheader( blocksize_, address_ );
      munmap( address_, fileheader.getBlock().blocksize * fileheader.getBlock().blocks );
    }
    if ( fd_ ) close( fd_ );
  }

  SystemError KVStore::open( const std::filesystem::path &path, ShareMode mode ) {
    fd_ = ::open( path.c_str(), O_RDWR | O_DSYNC, 0600 );
    if ( fd_ == -1 ) throw_SystemException( "open of file '" << path << "' failed", errno );
    ssize_t size = getFileSize( path );
    switch ( mode ) {
      case ShareMode::Private:
        address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_, 0 );
        waitReadLock_ = &KVStore::waitReadLockPrivate;
        waitWriteLock_ = &KVStore::waitWriteLockPrivate;
        break;
      case ShareMode::Shared:
        address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0 );
        waitReadLock_ = &KVStore::waitReadLockShared;
        waitWriteLock_ = &KVStore::waitWriteLockShared;
        break;
      case ShareMode::Clustered:
        address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0 );
        waitReadLock_ = &KVStore::waitReadLockShared;
        waitWriteLock_ = &KVStore::waitWriteLockShared;
        break;
    }
    if ( address_ == MAP_FAILED ) throw_SystemException( "mmap failed", errno );
    return SystemError::ecOK;
  }

  SystemError KVStore::init( const std::filesystem::path &path, size_t blocksize, size_t blocks, ShareMode mode ) {
    fd_ = ::open( path.c_str(), O_RDWR | O_DSYNC | O_CREAT | O_TRUNC, 0600 );
    if ( fd_ == -1 ) throw_SystemException( "creation of file '" << path << "' failed", errno );
    // size the file
    blocksize_ = ( blocksize / sysconf(_SC_PAGESIZE) ) * sysconf(_SC_PAGESIZE);
    size_t l_blocks = blocks;
    if ( l_blocks < min_blocks_ ) l_blocks = min_blocks_;
    size_t size = blocksize_ * l_blocks;
    lseek( fd_, size - 1, SEEK_SET );
    write( fd_, "A", 1 );
    switch ( mode ) {
      case ShareMode::Private:
        address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_, 0 );
        break;
      case ShareMode::Shared:
      case ShareMode::Clustered:
        address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0 );
        break;
    }
    if ( address_ == MAP_FAILED ) throw_SystemException( "mmap failed", errno );

    // initialize header
    FileHeader fileheader( blocksize_, address_ );
    fileheader.init( l_blocks );
    fileheader.setInfo( "Dodo test name", "Dodo test description", "Dodo test contact" );
    fileheader.getBlock().block_header.syncCRC32( blocksize_ );

    // initialize TOC
    TOC toc( blocksize_, getBlockAddress( 1 ) );
    toc.init( 1 );
    toc.setEntry( 0, btFileHeader );
    toc.setEntry( 1, btTOC );
    toc.setEntry( 2, btIndex );
    toc.getBlock().block_header.syncCRC32( blocksize_ );

    // initialize blocks
    for ( BlockId b = 2; b < fileheader.getBlock().blocks; b++ ) {
      BlockHeader* bh = getBlockAddress( b );
      bh->zero(blocksize_);
      bh->blockid = b;
      bh->blocktype = btFree;
    }

    SystemError rc = msync( address_, size, MS_SYNC );

    if ( rc != 0 ) throw_SystemException( "msync failed", errno );
    return SystemError::ecOK;
  }

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
        os_ << msg << endl;
        return ok_;
      }
    private:
      /** Output stream */
      std::ostream &os_;
      /** Test result. */
      bool ok_;
  };

  bool KVStore::analyze( std::ostream &os ) {
    bool ok = true;
    os << "FileHeader" << endl;
    FileHeader fileheader( blocksize_, address_ );

    Tester tester( os );
    ok = tester.test( common::Puts() << "blocktype " << fileheader.getBlock().block_header.blocktype,
      [&fileheader](){ return fileheader.getBlock().block_header.blocktype == BlockType::btFileHeader; } );

    ok = tester.test( common::Puts() << "magic " << fileheader.getBlock().magic,
      [&fileheader,this](){ return fileheader.getBlock().magic == FileHeader::magic; } );

    ok = tester.test( common::Puts() << "blocksize " << fileheader.getBlock().blocksize,
      [&fileheader](){ return fileheader.getBlock().blocksize % sysconf(_SC_PAGESIZE) == 0; } );

    ok = tester.test( common::Puts() << "file version " << fileheader.getBlock().version,
      [&fileheader,this](){ return fileheader.getBlock().version = FileHeader::version; } );

    ok = tester.test( common::Puts() << "crc32 0x" << common::Puts::hex() << fileheader.getBlock().block_header.calcCRC32( blocksize_ ) << common::Puts::dec(),
      [&fileheader,this](){ return fileheader.getBlock().block_header.calcCRC32( blocksize_ ) == fileheader.getBlock().block_header.crc32 ; } );

    struct stat st;
    fstat( fd_, &st );
    ok = tester.test( common::Puts() << "file size " << st.st_size,
      [&fileheader,st,this](){ return fileheader.getBlock().blocksize * fileheader.getBlock().blocks == static_cast<uint64_t>( st.st_size ); } );

    os << "created " << fileheader.getBlock().created << std::endl;
    os << "blocks " << fileheader.getBlock().blocks << std::endl;
    std::string name, description, contact;
    fileheader.getInfo( name, description, contact );
    os << "name " << name << std::endl;
    os << "description " << description << std::endl;
    os << "contact " << contact << std::endl;

    return ok;
  }

}