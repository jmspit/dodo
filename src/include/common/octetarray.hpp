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

namespace dodo::common {

  /**
   * Octet aka 'byte'.
   */
  typedef uint8_t Octet;

  /**
   * An array of Octets with size elements. Provides base64 conversion, random data, and appending of data form
   * other sources. Memory management is implicit, but allocated in chunks of size alloc_block to avoid
   * calling realloc on every size increase.
   */
  struct OctetArray {

    /**
     * Construct an empty OctetArray.
     */
    OctetArray() : array(nullptr), size(0), allocated_size(0) {}

    /**
     * Construct and fill from a std::string, not including a NULL terminator (so size will be string.length() ).
     * @param s The source string.
     */
    OctetArray( const std::string &s ) : array(nullptr), size(0), allocated_size(0) {
      *this = s;
    }

    /**
     * Destruct and clean.
     */
    virtual ~OctetArray() { this->free(); }

    /**
     * The Octet array.
     */
    Octet* array;

    /**
     * The array size in Octets (using an int as that is what opensssl uses)
     */
    size_t size;

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
     * @param sz The size in bytes to alloc .
     */
    void reserve( size_t sz );

    /**
     * Free and clear data.
     */
    void free();

    /**
     * The way in which two OctetArray instances match.
     */
    enum class MatchType {
      Mismatch,
      Partial,
      Full
    };

    /**
     * Match this OctetArray from index to the other array (entirely).
     * @param other The other OctetArray.
     * @param index The start index into the local array to match from.
     * @param octets The number of Octets in this array that match.
     * @return
     *   - Mismatch : The local array does not match the other array.
     *   - Partial : The local array matches the other array, but the other array is larger.
     *   - Full : The other array matches (contains) the local array completely, although the local array
     *             may have more octets.
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

    private:
      /**
       * The allocated size, always >= size.
       */
      size_t allocated_size;

  };

}

#endif