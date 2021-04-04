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
 * @file bytes.cpp
 * Implements the dodo::common::Bytes class.
 */

#include <cstring>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "common/exception.hpp"
#include "common/bytes.hpp"

namespace dodo::common {

  void Bytes::reserve( size_t sz ) {
    size_t chunkedsz = ( sz / alloc_block + 1 ) * alloc_block;
    if ( sz == size_ ) return;
    if ( array_ ) {
      if ( sz > allocated_size ) {
        array_ = static_cast< Octet*>( std::realloc( array_, chunkedsz ) );
        if ( array_ || sz == 0 ) {
          size_ = sz;
          allocated_size = chunkedsz;
          if ( !sz ) array_ = nullptr;
        } else throw_SystemException( "realloc of " << sz << " bytes failed", errno );
      } else {
        size_ = sz;
      }
    } else {
      array_ = static_cast< Octet*>( std::malloc( chunkedsz ) );
      if ( array_ ) {
        size_ = sz;
        allocated_size = chunkedsz;
      } else throw_SystemException( "malloc of " << sz << " bytes failed", errno );
    }
  }

  void Bytes::free() {
    if ( array_ != nullptr ) {
      std::free( array_ );
      array_ = nullptr;
    }
    size_ = 0;
    allocated_size = 0;
  }

  void Bytes::append( const Bytes& src ) {
    size_t osize = size_;
    this->reserve( size_ + src.size_ );
    memcpy( array_ + osize, src.array_, src.size_ );
  }

  void Bytes::append( const Bytes& src, size_t n ) {
    if ( n > src.size_ ) throw_Exception( "cannot append more than available in src" );
    size_t osize = size_;
    this->reserve( size_ + n );
    memcpy( array_ + osize, src.array_, n );
  }

  void Bytes::append( const Octet* src, size_t n ) {
    size_t osize = size_;
    this->reserve( size_ + n );
    memcpy( array_ + osize, src, n );
  }

  void Bytes::append( Octet src ) {
    this->reserve( size_ + 1 );
    array_[size_ - 1 ] = src;
  }

  Bytes& Bytes::operator=( const std::string &s ) {
    this->reserve( s.length() );
    for ( size_t i = 0; i < s.length(); i++ ) {
      array_[i] = static_cast<Octet>( s[i] );
    }
    return *this;
  }

  //Bytes::operator std::string() const {
  std::string Bytes::asString() const {
    std::stringstream ss;
    for ( size_t i = 0; i < size_; i++ ) {
      if ( array_[i] != 0 ) ss << array_[i];
      else if ( i != size_ -1 ) throw_Exception( "Bytes contain an intermediate zero - string convesrion failed" );
    }
    return ss.str();
  }

  void Bytes::random( size_t octets ) {
    if ( octets != size_ ) {
      this->reserve( octets );
    }
    RAND_bytes( array_, (int)size_ );
  }


  Bytes& Bytes::decodeBase64( const std::string& src ) {
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
    memcpy( array_, temp, actsize );
    std::free( temp );
    return *this;
  }

  std::string Bytes::encodeBase64() const {
    size_t base64sz = size_;
    base64sz = (base64sz / 48 + 1)*65+1;
    unsigned char* target = (unsigned char*)std::malloc( base64sz );

    EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
    if ( !ctx ) throw_Exception( "EVP_ENCODE_CTX_new failed" );
    EVP_EncodeInit( ctx );
    int len = 0;
    int rc = EVP_EncodeUpdate( ctx, target, &len, array_, (int)size_ );
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

  Bytes::MatchType Bytes::match( const Bytes& other, size_t index, size_t &octets  ) {
    size_t local_size = size_ - index;
    size_t match_size = std::min( local_size, other.size_ );
    int cmp = memcmp( array_, other.array_, match_size );
    if ( cmp == 0 ) {
      if ( local_size < other.size_ ) {
        octets = local_size;
        return MatchType::Contained;
      } else if ( local_size > other.size_ ) {
        octets = other.size_;
        return MatchType::Contains;
      } else {
        octets = local_size;
        return MatchType::Full;
      }
    } else {
      octets = 0;
      return MatchType::Mismatch;
    }
  }

  std::string Bytes::hexDump( size_t n ) const {
    std::stringstream ss;
    for ( size_t i = 0; i < size_ && i < n; i++ ) {
      if ( i != 0 ) ss << ":";
      ss << std::setw(2) << std::hex << std::setfill('0') << (unsigned int)array_[i];
    }
    return ss.str();
  }

}