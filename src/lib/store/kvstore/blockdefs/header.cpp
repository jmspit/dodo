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
 * @file common.cpp
 * Implements the dodo::store::kvstore::FileHeader class.
 */

#include <store/kvstore/blockdefs/header.hpp>

#include <cstring>

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

  void FileHeader::init( uint64_t blocks ) {
    block_->block_header.init( blocksize_, 0, btFileHeader );
    block_->blocksize = blocksize_;
    block_->blocks = blocks;
    block_->magic = magic;
    block_->version = version;
    block_->created = time(nullptr);
    block_->name_sz = 0;
    block_->description_sz = 0;
    block_->contact_sz = 0;    
  }


}