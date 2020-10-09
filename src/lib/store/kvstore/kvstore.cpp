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
#include <filesystem>
#include <mutex>
#include <shared_mutex>

#include <common/util.hpp>
#include <common/exception.hpp>

#include <cassert>


namespace dodo::store::kvstore {

  ReadBlockLock::ReadBlockLock( BlockId blockid ) {
    KVStore* kvstore = KVStore::getKVStore();
    fd_ = kvstore->getFD();
    lock_.l_type = F_RDLCK;
    lock_.l_whence = SEEK_SET;
    lock_.l_start = 0;
    lock_.l_len = 1024;
    fcntl( fd_, F_OFD_SETLKW, &lock_ );
  }

  ReadBlockLock::~ReadBlockLock() {
    lock_.l_type = F_UNLCK;
    fcntl( fd_, F_OFD_SETLKW, &lock_ );
  }

  WriteBlockLock::WriteBlockLock( BlockId blockid ) {
    KVStore* kvstore = KVStore::getKVStore();
    fd_ = kvstore->getFD();
    lock_.l_type = F_WRLCK;
    lock_.l_whence = SEEK_SET;
    lock_.l_start = 0;
    lock_.l_len = 1024;
    fcntl( fd_, F_OFD_SETLKW, &lock_ );
  }

  WriteBlockLock::~WriteBlockLock() {
    lock_.l_type = F_UNLCK;
    fcntl( fd_, F_OFD_SETLKW, &lock_ );
  }


  KVStore::KVStore() {
    blocksize_ = static_cast<BlockSize>( sysconf(_SC_PAGESIZE) );
    assert( sizeof(BlockHeader) == 16 );
    assert( sizeof(FileHeader::BlockDef) == 52 );
    assert( sizeof(TOC::BlockDef) == 44 );
    assert( sizeof(Data::BlockDef) == 48 );
    assert( sizeof(Data::RowEntry) == 24 );
  }

  KVStore::~KVStore() {
    std::unique_lock lock( big_mutex_ );
    if ( address_ ) {
      FileHeader fileheader( this, address_ );
      munmap( address_, fileheader.getBlock().blocksize * fileheader.getBlock().blocks );
      address_ = nullptr;
    }
    for ( const auto &fd : fd_map_ ) {
      close( fd.second );
    }
  }

  KVStore* KVStore::singleton_ = nullptr;

  KVStore* KVStore::getKVStore() {
    if ( !singleton_ ) singleton_ = new KVStore();
    return singleton_;
  }

  SystemError KVStore::open( const std::filesystem::path &path ) {
    path_ = path;
    int fd = attachThread();
    if ( fd == -1 ) throw_SystemException( "open of file '" << path_ << "' failed", errno );
    std::unique_lock big_lock( big_mutex_ );
    fd_map_[ std::this_thread::get_id() ] = fd;
    FileSize size = getFileSize( path );
    address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );
    if ( address_ == MAP_FAILED ) throw_SystemException( "mmap failed", errno );
    return SystemError::ecOK;
  }

  int KVStore::getFD() {
    shared_lock<shared_timed_mutex> r(fd_map_mutex_);

    auto i = fd_map_.find( std::this_thread::get_id() );
    if ( i == fd_map_.end() ) return -1; else return i->second;
  }

  int KVStore::getThisThreadFD() {
    auto i = fd_map_.end();
    {
      std::shared_lock lock(fd_map_mutex_);
      i = fd_map_.find( std::this_thread::get_id() );
    }
    if ( i == fd_map_.end() ) return attachThread(); else return i->second;
  }

  SystemError KVStore::init( const std::filesystem::path &path, BlockSize blocksize, size_t blocks ) {
    std::unique_lock big_lock( big_mutex_ );
    path_ = path;
    std::filesystem::space_info dev = std::filesystem::space(path);
    BlockSize spsz = static_cast<BlockSize>( sysconf(_SC_PAGESIZE) );
    blocksize_ = ( blocksize / spsz ) * spsz;
    size_t blocks_ = blocks;
    if ( blocks_ < min_blocks_ ) blocks_ = min_blocks_;
    FileSize size = blocksize_ * blocks_;
    if ( size > dev.available-1024*1024*10 ) throw_Exception( "insufficient filesystem free space, " << size << " bytes required" );

    int fd = ::open( path_.c_str(), O_RDWR | O_DSYNC | O_CREAT | O_TRUNC, 0600 );
    fd_map_[ std::this_thread::get_id() ] = fd;
    if ( fd == -1 ) throw_SystemException( "creation of file '" << path_ << "' failed", errno );
    // size the file
    lseek( fd, size - 1, SEEK_SET );
    write( fd, "A", 1 );
    address_ = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );
    if ( address_ == MAP_FAILED ) throw_SystemException( "mmap failed", errno );

    // initialize header
    FileHeader fileheader( this, address_ );
    fileheader.init( blocks_ );
    fileheader.setInfo( "Dodo test name", "Dodo test description", "Dodo test contact" );
    fileheader.getBlock().block_header.syncCRC32( blocksize_ );

    // initialize TOC
    TOC toc( this, getBlockAddress( 1 ) );
    toc.init( 1, 0 );
    toc.setEntry( 0, btFileHeader );
    toc.setEntry( 1, btTOC );
    toc.setEntry( 2, btIndexTree );
    toc.setEntry( 3, btData );

    // initialize index root block
    IndexTree idxt( blocksize_, getBlockAddress( 2 ) );
    idxt.init( 2 );
    idxt.getBlock().block_header.syncCRC32( blocksize_ );

    // initialize one data block
    Data dat( this, getBlockAddress( 3 ) );
    dat.init( 3 );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0000") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0001") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0002") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0003") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0004") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0005") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0006") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0007") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0008") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0009") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0010") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0011") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0012") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0013") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0014") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0015") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0016") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0017") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0018") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0019") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0020") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0021") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0022") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0023") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0024") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0025") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0026") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0027") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0028") );
    dat.allocateRow( common::OctetArray("texttexttexttextexttexttexttext0029") );

    dat.getBlock().block_header.syncCRC32( blocksize_ );


    // initialize remaining blocks
    for ( BlockId b = 4; b < fileheader.getBlock().blocks; b++ ) {
      BlockHeader* bh = getBlockAddress( b );
      bh->zero(blocksize_);
      bh->blockid = b;
      bh->blocktype = btFree;
      bh->syncCRC32( blocksize_ );
      toc.setEntry( b, btFree );
    }

    toc.getBlock().block_header.syncCRC32( blocksize_ );

    SystemError rc = msync( address_, size, MS_SYNC );

    if ( rc != 0 ) throw_SystemException( "msync failed", errno );
    return SystemError::ecOK;
  }

  int KVStore::attachThread() {
    {
      std::shared_lock lock( fd_map_mutex_ );
      auto i = fd_map_.find( std::this_thread::get_id() );
      if ( i != fd_map_.end() ) return i->second;
    }
    int fd = ::open( path_.c_str(), O_RDWR | O_DSYNC, 0600 );
    if ( fd == -1 ) throw_SystemException( "open of file '" << path_ << "' failed", errno );
    {
      std::unique_lock lock( fd_map_mutex_ );
      fd_map_[ std::this_thread::get_id() ] = fd;
    }
    return fd;
  }

  void KVStore::releaseThread() {
    {
      std::unique_lock lock( fd_map_mutex_ );
      fd_map_.erase( std::this_thread::get_id() );
    }
  }

  void KVStore::extend( BlockId blocks ) {
    int fd = getThisThreadFD();
    std::unique_lock big_lock( big_mutex_ );
    // resize the file
    FileHeader fileheader1( this, address_ );
    FileSize oldsize = blocksize_ * fileheader1.getBlock().blocks;
    FileSize size = blocksize_ * (fileheader1.getBlock().blocks + blocks);
    ftruncate( fd, size );
    void* relocated = mremap( address_, oldsize, size, MREMAP_MAYMOVE );
    if ( relocated != MAP_FAILED ) address_ = relocated; else throw_SystemException( "mremap failed", errno );

    TOC toc( this, getBlockAddress( 1 ) );
    FileHeader fileheader2( this, address_ );
    for ( BlockId b = fileheader2.getBlock().blocks; b < fileheader2.getBlock().blocks + blocks; b++ ) {
      toc.setEntry( b, btFree );
    }

    // //TOC toc( blocksize_, getBlockAddress( 1 ) ); // @todo  - could be the wrong TOC

    fileheader2.getBlock().blocks += blocks;
    fileheader2.getBlock().block_header.syncCRC32(blocksize_);
    toc.getBlock().block_header.syncCRC32(blocksize_);
  }

  bool KVStore::analyze( std::ostream &os ) {
    std::unique_lock big_lock( big_mutex_ );

    for ( const auto &fd : fd_map_ ) {
      cout << "thread id " << fd.first << " file descriptor " << fd.second << endl;
    }

    FileHeader fileheader( this, address_ );
    bool ok = fileheader.analyze( os );

    TOC toc( this, getBlockAddress( 1 ) );
    ok = toc.analyze( os );

    Data data( this, getBlockAddress( 3 ) );
    ok = ok && data.analyze( os );
    data.releaseRow( 6 );
    data.getBlock().block_header.syncCRC32( blocksize_ );
    ok = ok && data.analyze( os );
    data.allocateRow( common::OctetArray( "replace6" ) );
    data.getBlock().block_header.syncCRC32( blocksize_ );
    ok = ok && data.analyze( os );
    return ok;
  }

}