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
 * @file octetarray.cpp
 * Implements the dodo::common::OctetArray class..
 */

#include "common/exception.hpp"
#include "common/octetarray.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>

namespace dodo::common {

  void OctetArray::reserve( size_t sz ) {
    if ( sz == size ) return;
    if ( array ) {
      array = static_cast< Octet*>( std::realloc( array, sz ) );
      if ( array || sz == 0 ) {
        size = sz;
        if ( !sz ) array = nullptr;
      } else throw_SystemException( Puts() << "realloc of " << sz << " bytes failed", errno );
    } else {
      array = static_cast< Octet*>( std::malloc( sz ) );
      if ( array ) {
        size = sz;
      } else throw_SystemException( Puts() << "malloc of " << sz << " bytes failed", errno );
    }
  }

  void OctetArray::free() {
    if ( array != nullptr ) {
      std::free( array );
      array = nullptr;
    }
    size = 0;
  }

  void OctetArray::append( const OctetArray& src ) {
    size_t osize = size;
    this->reserve( size + src.size );
    memcpy( array + osize, src.array, src.size );
  }

  void OctetArray::append( const OctetArray& src, size_t n ) {
    if ( n > src.size ) throw_Exception( "cannot append more than available in src" );
    size_t osize = size;
    this->reserve( size + n );
    memcpy( array + osize, src.array, n );
  }

  void OctetArray::append( const Octet* src, size_t n ) {
    size_t osize = size;
    this->reserve( size + n );
    memcpy( array + osize, src, n );
  }

  OctetArray& OctetArray::operator=( const std::string &s ) {
    this->reserve( s.length() );
    for ( size_t i = 0; i < s.length(); i++ ) {
      array[i] = static_cast<Octet>( s[i] );
    }
    return *this;
  }

  OctetArray::operator std::string() const {
    std::stringstream ss;
    for ( size_t i = 0; i < size; i++ ) {
      if ( array[i] != 0 ) ss << array[i];
    }
    return ss.str();
  }

  void OctetArray::random( size_t octets ) {
    if ( octets != size ) {
      this->reserve( octets );
    }
    RAND_bytes( array, (int)size );
  }


  OctetArray& OctetArray::decodeBase64( const std::string& src ) {
    Octet* temp = (Octet*)std::malloc( src.length() ); // too much, but safe, and temp anyway


    EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
    if ( !ctx ) throw_Exception( "EVP_ENCODE_CTX_new failed" );
    EVP_DecodeInit( ctx );
    int len = 0;
    int rc = EVP_DecodeUpdate( ctx, temp, &len, (const unsigned char*)src.c_str(), (int)src.length() );
    if ( rc == -1 ) throw_Exception( "EVP_DecodeUpdate failed" );
    int actsize = len;
    rc = EVP_DecodeFinal( ctx, temp + len, &len );
    if ( rc != 1 ) throw_Exception( "EVP_DecodeFinal failed" );
    actsize += len;
    EVP_ENCODE_CTX_free( ctx );

    this->reserve( actsize );
    memcpy( array, temp, actsize );
    std::free( temp );
    return *this;
  }

  std::string OctetArray::encodeBase64() const {
    size_t base64sz = size;
    base64sz = (base64sz / 48 + 1)*65+1;
    unsigned char* target = (unsigned char*)std::malloc( base64sz );

    EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
    if ( !ctx ) throw_Exception( "EVP_ENCODE_CTX_new failed" );
    EVP_EncodeInit( ctx );
    int len = 0;
    int rc = EVP_EncodeUpdate( ctx, target, &len, array, (int)size );
    if ( rc != 1 ) throw_Exception( "EVP_EncodeUpdate failed" );
    int actsize = len;
    EVP_EncodeFinal( ctx, target + actsize, &len );
    actsize += len;

    EVP_ENCODE_CTX_free( ctx );
    std::stringstream ss;
    for ( int i = 0; i < actsize; i++ ) {
      char c = static_cast<unsigned char>( target[i] );
      if ( c && c != '\n' ) ss << c;
    }

    std::free( target );
    return ss.str();
  }

}