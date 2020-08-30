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

#include <store/kvstore/blockdefs/common.hpp>

#include <string.h>

namespace dodo::store::kvstore {

  uint32_t BlockHeader::calcCRC32( size_t blocksize ) {
    uint8_t* address = (uint8_t*)this;
    uint32_t crc = crc32_fast( address,
                               sizeof(blockid)+sizeof(blocktype),
                               0 );
    crc = crc32_fast( address +  sizeof(blockid)+sizeof(blocktype)+sizeof(crc32),
                      blocksize-sizeof(blockid)-sizeof(blocktype)-sizeof(crc32),
                      crc );
    return crc;
  }

  void BlockHeader::syncCRC32( size_t blocksize ) {
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

  void BlockHeader::zero( size_t blocksize ) {
    uint8_t* address = (uint8_t*)this;
    memset( address, 0, blocksize );
  }

}