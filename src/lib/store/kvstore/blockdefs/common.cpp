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
 * Implements dodo::store::KVStore common things
 */

#include <cstring>

#include "store/kvstore/blockdefs/common.hpp"

namespace dodo::store::kvstore {

  uint32_t BlockHeader::calcCRC32( BlockSize blocksize ) const {
    uint8_t* address = (uint8_t*)this;
    uint32_t crc = crc32_fast( address,
                               sizeof(blockid)+sizeof(blocktype),
                               0 );
    crc = crc32_fast( address +  sizeof(blockid)+sizeof(blocktype)+sizeof(crc32),
                      blocksize-sizeof(blockid)-sizeof(blocktype)-sizeof(crc32),
                      crc );
    return crc;
  }

  void BlockHeader::syncCRC32( BlockSize blocksize ) {
    uint8_t* address = (uint8_t*)this;
    uint32_t crc = crc32_fast( address,
                               sizeof(blockid)+sizeof(blocktype),
                               0 );
    crc = crc32_fast( address +  sizeof(blockid)+sizeof(blocktype)+sizeof(crc32),
                      blocksize-sizeof(blockid)-sizeof(blocktype)-sizeof(crc32),
                      crc );
    BlockHeader *hdr = reinterpret_cast<BlockHeader*>( address );
    hdr->crc32 = crc;
  }

  bool BlockHeader::verifyCRC32( BlockSize blocksize ) const {
    uint8_t* address = (uint8_t*)this;
    BlockHeader *hdr = reinterpret_cast<BlockHeader*>( address );
    return calcCRC32( blocksize ) == hdr->crc32;
  }

  void BlockHeader::zero( BlockSize blocksize ) {
    uint8_t* address = (uint8_t*)this;
    memset( address, 0, blocksize );
  }

  void GenericBlock::outputHeader( std::ostream& os, const std::string &caption ) {
    os << std::endl;
    os << std::string( 80, '-' ) << std::endl;
    os << caption << std::endl;
    os << std::string( 80, '-' ) << std::endl;
  }

}