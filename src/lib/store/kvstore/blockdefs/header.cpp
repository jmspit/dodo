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
 * @file header.cpp
 * Implements the dodo::store::kvstore::FileHeader class.
 */

#include <store/kvstore/kvstore.hpp>
#include <store/kvstore/blockdefs/header.hpp>
#include <common/puts.hpp>

#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace dodo::store::kvstore {

  void FileHeader::setInfo( const std::string &name,
                            const std::string &description,
                            const std::string &contact ) {
    std::string s_name = name.substr( 0, 256 );
    std::string s_description = description.substr( 0, 2048 );
    std::string s_contact = contact.substr( 0, 256 );
    uint16_t* p = &(block_->name_sz);
    *p = static_cast<uint16_t>( s_name.length() );
    p = &(block_->description_sz);
    *p = static_cast<uint16_t>( s_description.length() );
    p = &(block_->contact_sz);
    *p = static_cast<uint16_t>( s_contact.length() );
    uint8_t *d =  reinterpret_cast<uint8_t*>( &(block_->contact_sz) ) + sizeof(block_->contact_sz);
    memcpy( d, s_name.c_str(), s_name.length() );
    d += s_name.length();
    memcpy( d, s_description.c_str(), s_description.length() );
    d += s_description.length();
    memcpy( d, s_contact.c_str(), s_contact.length() );
    d += s_contact.length();
  }

  void FileHeader::getInfo( std::string &name,
                            std::string &description,
                            std::string &contact ) {
    char *d =   reinterpret_cast<char*>( &(block_->contact_sz) ) + sizeof(block_->contact_sz);
    name = std::string( d, block_->name_sz );
    d += block_->name_sz;
    description = std::string( d, block_->description_sz );
    d += block_->description_sz;
    contact = std::string( d, block_->contact_sz );
  }

  void FileHeader::init( BlockCount blocks ) {
    block_->block_header.init( store_->getBlockSize(), 0, btFileHeader );
    block_->blocksize = store_->getBlockSize();
    block_->blocks = blocks;
    block_->magic = magic;
    block_->version = version;
    block_->created = time(nullptr);
    block_->name_sz = 0;
    block_->description_sz = 0;
    block_->contact_sz = 0;
  }

  bool FileHeader::analyze( std::ostream &os ) {
    bool ok = true;
    outputHeader( os, "Type sizes (bytes)" );
    os << "BlockId " << sizeof(BlockId) << std::endl;
    os << "RowId " << sizeof(RowId) << std::endl;
    os << "BlockSize " << sizeof(BlockSize) << std::endl;
    os << "BlockCount " << sizeof(BlockCount) << std::endl;
    os << "BlockType " << sizeof(BlockType) << std::endl;
    os << "FileSize " << sizeof(FileSize) << std::endl;
    os << "BlockHeader " << sizeof(BlockHeader) << std::endl;
    os << "TOC::BlockDef " << sizeof(TOC::BlockDef) << std::endl;
    os << "Data::BlockDef " << sizeof(Data::BlockDef) << std::endl;
    os << "Data::RowEntry " << sizeof(Data::RowEntry) << std::endl;

    outputHeader( os, "Limits" );
    os << "maximum key size " << (block_->blocksize-sizeof(IndexLeaf::BlockDef)-sizeof(IndexLeaf::IndexEntry))/2 << std::endl;
    os << "maximum 1 byte rows per block " << block_->blocksize-sizeof(Data::BlockDef) << std::endl;

    outputHeader( os, "File header block" );
    Tester tester( os );
    ok = tester.test( common::Puts() << "blockid " << block_->block_header.blockid,
      [this](){ return block_->block_header.blockid == 0; } );

    ok = tester.test( common::Puts() << "blocktype " << block_->block_header.blocktype,
      [this](){ return block_->block_header.blocktype == BlockType::btFileHeader; } );

    ok = tester.test( common::Puts() << "magic " << block_->magic,
      [this](){ return block_->magic == FileHeader::magic; } );

    ok = tester.test( common::Puts() << "blocksize " << block_->blocksize << " is integer multiple of system page size ("
                                     << block_->blocksize / sysconf(_SC_PAGESIZE) << "x" << sysconf(_SC_PAGESIZE)  << ") ",
                      [this](){ return block_->blocksize % sysconf(_SC_PAGESIZE) == 0; } );

    ok = tester.test( common::Puts() << "file version " << block_->version,
      [this](){ return block_->version = FileHeader::version; } );

    ok = tester.test( common::Puts() << "crc32 0x" << common::Puts::hex() << block_->block_header.calcCRC32( store_->getBlockSize() ) << common::Puts::dec(),
      [this](){ return block_->block_header.verifyCRC32( store_->getBlockSize() ) ; } );

    struct stat st;
    fstat( store_->getFD(), &st );
    ok = tester.test( common::Puts() << "file size " << st.st_size,
      [st,this](){ return block_->blocksize * block_->blocks == static_cast<uint64_t>( st.st_size ); } );

    os << "created " << block_->created << std::endl;
    os << "blocks " << block_->blocks << std::endl;
    std::string name, description, contact;
    getInfo( name, description, contact );
    os << "name " << name << std::endl;
    os << "description " << description << std::endl;
    os << "contact " << contact << std::endl;

    return ok;
  }


}