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
   * other sources. Memory management is implicit.
   */
  struct OctetArray {

    /**
     * Construct an empty OctetArray.
     */
    OctetArray() : array(nullptr), size(0) {}

    /**
     * Construct from a std::string.
     * @param s The source string.
     */
    OctetArray( const std::string &s ) : array(nullptr), size(0) {
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
    size_t  size;

    /**
     * Assign from a base64 std::string. Decodes the data to the octet array.
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
    operator std::string() const;

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
     * Generate a random set of Octets.
     * @param octets The number of random Octets.
     */
    void random( size_t octets );

  };

}

#endif