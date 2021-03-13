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
 * @file octetarray.hpp
 * Defines the dodo::common::OctetArray class..
 */

#ifndef common_octetarray_hpp
#define common_octetarray_hpp

#include <string>
#include <cassert>

namespace dodo::common {

  /**
   * Octet aka 'byte'.
   */
  typedef uint8_t Octet;

  /**
   * An array of Octets with size elements. Provides base64 conversion, random data generation, appending of data from
   * various other sources. Memory management is implicit, resizing allocates at least a chunk.
   *
   * Note that OctetArray objects are not thread safe, and that in general the pointer returned by getArray() may
   * be invalidated by subjequent calls to append(), which could possibly realloc memory, invalidating the previously
   * returned pointer.
   */
  class OctetArray {

    public:

      /**
       * Construct an empty OctetArray.
       */
      OctetArray() : array_(nullptr), size_(0), allocated_size(0) {}

      /**
       * Construct an OctetArray by taking ownership of existing data (which will be freed when this object is destructed).
       * @param data The data.
       * @param size The size of the data.
       */
      OctetArray( Octet* data, size_t size ) {
        array_ = data;
        size = size;
      }

      /**
       * Construct and fill from a std::string, not including the NULL terminator (so size will be string.length() ).
       * @param s The source string.
       */
      OctetArray( const std::string &s ) : array_(nullptr), size_(0), allocated_size(0) {
        *this = s;
      }

      /**
       * Destruct and clean.
       */
      virtual ~OctetArray() { this->free(); }

      /**
       * Assign from std::string, not including a NULL terminator (so size will be string.length() ).
       * @param s The base64 string to assign from.
       * @return This OctetArray.
       */
      OctetArray& operator=( const std::string &s );

      /**
       * Decodes the base64 string into this OctetArray.
       * @param src The base64-encoded source string.
       * @return A reference to this OctetArray
       */
      OctetArray& decodeBase64( const std::string& src );

      /**
       * Encodes the this OctetArray into a base64-encoded string.
       * @return The base64 encoded data of the OctetArray.
       */
      std::string encodeBase64() const;

      /**
       * Cast to a std::string. Be aware that the string is read to
       * either the first zero (NULL) or up to size. So if the decoded data
       * contains intermediate zeros the string will not cover all octets.
       * @return A string representation of OctetArray.
       */
      //operator std::string() const;

      std::string asString() const;

      /**
       * Reserve memory in the OctetArray.
       * @param size The size in bytes to allocate .
       */
      void reserve( size_t size );

      /**
       * Free and clear data.
       */
      void free();

      /**
       * The way in which two OctetArray instances match.
       */
      enum class MatchType {
        Mismatch,   /**< The local array does not match the other array. */
        Contained,  /**< The local array is contained in the other array, but the other array has more data. */
        Contains,   /**< The local array contains the other array, but the local array has more data. */
        Full        /**< The local and other array are equal in content and size. */
      };

      /**
       * Match this OctetArray from index to the other array (entirely).
       * @param other The other OctetArray.
       * @param index The start index into the local array to match from.
       * @param octets The number of Octets, from the start of the arrays, that match.
       * @return The MatchType.
       */
      MatchType match( const OctetArray& other, size_t index, size_t &octets );

      /**
       * Append another OctetArray.
       * @param src The OctetArray to append.
       */
      void append( const OctetArray& src );

      /**
       * Append the first n Octets from OctetArray.
       * @param src The OctetArray to append from.
       * @param n The number of octets to append.
       */
      void append( const OctetArray& src, size_t n );

      /**
       * Append from arbitrary memory areas.
       * @param src The memory to append from.
       * @param n The number of octets to append.
       */
      void append( const Octet* src, size_t n );

      /**
       * Append a single Octet
       * @param src The Octet to append.
       */
      void append( Octet src );

      /**
       * Generate a random set of Octets.
       * @param octets The number of random Octets.
       */
      void random( size_t octets );

      /**
       * hex dump the first n octets of the data.
       * @param n The number of octets to dump.
       * @return a string with hex numbers.
       */
      std::string hexDump( size_t n ) const;

      /**
       * Always allocate chunks of this size. Any OctetArray that has been called with reserve( n > 1 ) will
       * at least allocate 1 chunk of alloc_block bytes. Avoids frequent calls to realloc when using many append
       * calls with small amount of data such as append( Octet ).
       */
      const size_t alloc_block = 32;

      /**
       * Return the array. Note that this pointer may be invalidated by
       * subsequent calls to append (which implictly calls reserve, which may ralloc the memory block). Avoid using
       * this method.
       * @return a pointer to the array of Octets.
       */
      Octet* getArray() const { return array_; }

      /**
       * Return the array size.
       * @return the array size.
       */
      size_t getSize() const { return size_; }

      /**
       * Return the Octet at index. Only in debug builds the index is asserted to be within range.
       * @param index The index of the Octet
       * @return The Octet.
       */
      Octet getOctet( size_t index ) const { assert( index < size_ ); return array_[index]; }

    private:
      /**
       * The Octet array.
       */
      Octet* array_;

      /**
       * The array size in Octets (using an int as that is what OpenSSL uses)
       */
      size_t size_;

      /**
       * The allocated size, always >= size.
       */
      size_t allocated_size;

  };

}

#endif