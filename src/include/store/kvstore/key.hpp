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
 * @file key.hpp
 * Defines the dodo::store::Key class.
 */

#ifndef store_kvstore_key_hpp
#define store_kvstore_key_hpp

#include <string>

namespace dodo::store::kvstore {

  /**
   * The Key of a key-value pair. Keys are case-insensitive.
   */
  class Key {
    public:

      /**
       * Construct an ampty key.
       */
      Key() { key_ = ""; }

      /**
       * Construct from string.
       * @param other The string to assign.
       */
      Key( const std::string &other ) { key_ = other; toLower(); }

      /**
       * Destructor.
       */
      virtual ~Key();

      /**
       * Test (case insensitive) equality.
       * @param other The string to compare to.
       * @return True when the keys are equal.
       */
      bool operator==( const Key& other ) const { return key_ == other.key_; }

      /**
       * Test inequality.
       * @param other The string to compare to.
       * @return True when the keys are unequal.
       */
      bool operator!=( const Key& other ) const { return key_ != other.key_; }

      /**
       * Test less-than.
       * @param other The string to compare to.
       * @return True when this Key < other (case insensitive).
       */
      bool operator<( const Key& other ) const { return key_ < other.key_; }

      /**
       * Test greater-than.
       * @param other The string to compare to.
       * @return True when this Key > other (case insensitive).
       */
      bool operator>( const Key& other ) const { return key_ > other.key_; }

      /**
       * Assign another key.
       * @param other The other key.
       * @return A reference to this Key.
       */
      Key& operator=( const Key& other ) { key_ = other.key_; return *this; }

      /**
       * Assign a string, which is converted o lowercase.
       * @param value The string.
       * @return A reference to this Key.
       */
      Key& operator=( const std::string& value ) { key_ = value; toLower(); return *this; }

    protected:

      /** Convert key_ to lowercase. */
      void toLower();

      /** The lowercase key. */
      std::string key_;

  };

}

#endif