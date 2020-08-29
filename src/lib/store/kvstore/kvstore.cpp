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

#include <store/kvstore.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/mman.h>


namespace dodo::store {


  KVStore::KVStore() : fd_(0) {
    pagesize_ = sysconf(_SC_PAGESIZE);
  }

  KVStore::~KVStore() {
    if ( address_ ) munmap( address_, size_ );
    if ( fd_ ) close( fd_ );
  }

  SystemError KVStore::open( const std::filesystem::path &path ) {
    fd_ = ::open( path.c_str(), O_RDWR | O_DSYNC, 0600 );
    if ( fd_ == -1 ) return errno;
    return SystemError::ecOK;
  }

  SystemError KVStore::init( const std::filesystem::path &path, ssize_t size ) {
    fd_ = ::open( path.c_str(), O_RDWR | O_DSYNC | O_CREAT | O_TRUNC, 0600 );
    if ( fd_ == -1 ) return errno;
    // size the file
    blocks_ = (size / pagesize_ + (size % pagesize_ > 0) );
    if ( blocks_ < min_blocks_ ) blocks_ = min_blocks_;
    size_ = blocks_ * pagesize_;
    cout << size_ << endl;
    lseek( fd_, size_ - 1, SEEK_SET );
    write( fd_, "A", 1 );
    address_ = mmap( nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0 );
    if ( address_ == MAP_FAILED ) return errno;

    // initialize header
    Header *header = (Header*) address_;
    header->block_header.blockid = 0L;
    header->block_header.blocktype = FileHeader;
    header->magic = magic;
    header->version = version;
    header->blocks = blocks_;

    // initialize TOC
    TOCHeader* toc_header = reinterpret_cast<TOCHeader*>( getBlockAddress( 1 ) );
    toc_header->block_header.blockid = 1L;
    toc_header->block_header.blocktype = TOC;
    toc_header->next_toc = 0L;

    // initialize blocks
    for ( BlockId b = 2; b < blocks_; b++ ) {
      BlockHeader* bh = getBlockAddress( b );
      bh->blockid = b;
      bh->blocktype = Free;
    }

    SystemError rc = msync( address_, size_, MS_SYNC );

    if ( rc != 0 ) return errno;
    return SystemError::ecOK;
  }

  void KVStore::analyze( std::ostream &os ) {
    Header *header = (Header*) address_;
    if ( header->magic == magic ) {
      os << "magic ok" << std::endl;
    } else {
      os << "bad magic" << std::endl; return;
    }
    os << "software version " << version << " file version " << header->version << std::endl;
    os << "#blocks " << blocks_ << std::endl;

  }

}