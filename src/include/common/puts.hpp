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
 * @file puts.hpp
 * Defines the dodo::common::Puts class.
 */

#ifndef common_puts_hpp
#define common_puts_hpp

#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

namespace dodo::common {

  /**
   * Helper class to write strings in stream format, eg
   * \code
   * string s = common::Puts() << "integer: " << 3 << " double: " << common::Puts:setprecision(2) << 3.145;
   * \endcode
   * which is very convenient in, for example, throwing exceptions, where the throw_Exception macro already inserts
   *  the "Puts() <<" in the expression, so one can write
   * \code
   * throw Exception( "open failed with errorcode: " << errorcode );
   * \endcode
   *
   *
   */
  class Puts {
    public:

      /**
       * Mimics std::endl.
       */
     struct endl {
      };

      /**
       * Put the stream in floating point fixed-format mode.
       * @see operator<<( fixed )
       */
      struct fixed {
      };

      /**
       * Put the stream in floating point scientific-format mode.
       * @see operator<<( scientific )
       */
      struct scientific {
      };

      /**
       * Put the stream in decimal mode.
       * @see operator<<( dec )
       */
      struct dec {
      };

      /**
       * Put the stream in hexadecimal mode.
       * @see operator<<( hex )
       */
      struct hex {
      };

      /**
       * Put the stream in octal mode.
       * @see operator<<( oct )
       */
      struct oct {
      };


      /**
       * Set the width of things to w characters.
       * @see operator<<( setw w )
       */
      struct setw {
        /**
         * Construct with width.
         * @param w The width.
         */
        setw( int w ) : w_(w) {};

        /** The width. */
        int w_;
      };

      /**
       * Set the precision for floating point fixed format
       * @see opeartor<<( setprecision )
       */
      struct setprecision {
        /**
         * Construct with precision.
         * @param p The precision.
         */
        setprecision( int p ) : p_(p) {};
        /** The precision. */
        int p_;
      };

      /**
       * Constructor inits to fixed format for double with precision 3.
       */
      Puts() { ss_ <<  std::setprecision(3) << std::fixed; };

      /**
       * Append a STL string.
       * @param s The string.
       * @return This Puts.
       */
      const Puts& operator<<( const std::string& s ) const {
        ss_ << s;
        return *this;
      }

      /**
       * Append a C string.
       * @param s The const char*
       * @return This Puts.
       */
      const Puts& operator<<( const char* s ) const {
        ss_ << s;
        return *this;
      }

      /**
       * Append a char.
       * @param c The char
       * @return This Puts.
       */
      const Puts& operator<<( char c ) const {
        ss_ << c;
        return *this;
      }

      /**
       * Append an integer.
       * @param i The int
       * @return This Puts.
       */
      const Puts& operator<<( int i ) const {
        ss_ << i;
        return *this;
      }

      /**
       * Append a long.
       * @param l The long
       * @return This Puts.
       */
      const Puts& operator<<( long l ) const {
        ss_ << l;
        return *this;
      }

      /**
       * Append a long long.
       * @param l The long long
       * @return This Puts.
       */
      const Puts& operator<<( long long l ) const {
        ss_ << l;
        return *this;
      }

      /**
       * Append an unsigned integer.
       * @param i The unsigned integer to append.
       * @return This Puts.
       */
      const Puts& operator<<( unsigned int i ) const {
        ss_ << i;
        return *this;
      }

      /**
       * Append an unsigned long.
       * @param l The unsigned long to append.
       * @return This Puts.
       */
      const Puts& operator<<( unsigned long l ) const {
        ss_ << l;
        return *this;
      }

      /**
       * Append an unsigned long long.
       * @param l The unsigned long long to append.
       * @return This Puts.
       */
      const Puts& operator<<( unsigned long long l ) const {
        ss_ << l;
        return *this;
      }

      /**
       * Append a float.
       * @param f The float to append.
       * @return This Puts.
       */
      const Puts& operator<<( float f ) const {
        ss_ << f;
        return *this;
      }

      /**
       * Append a double.
       * @param d The double to append.
       * @return This Puts.
       */
      const Puts& operator<<( double d ) const {
        ss_ << d;
        return *this;
      }

      /**
       * Append a void*.
       * @param p The void* to append.
       * @return This Puts.
       */
      const Puts& operator<<( void *p ) const {
        ss_ << p;
        return *this;
      }

      /**
       * Append a std::thread::id.
       * @param id The std::thread::id to append.
       * @return This Puts.
       */
      const Puts& operator<<( std::thread::id id ) const {
        ss_ << id;
        return *this;
      }

      /**
       * Appends std::endl
       * @return This Puts.
       */
      const Puts& operator<<( Puts::endl ) const {
        ss_ << std::endl;
        return *this;
      }

      /**
       * Applies std::fixed
       * @return This Puts.
       */
      const Puts& operator<<( fixed ) const {
        ss_ << std::fixed;
        return *this;
      }

      /**
       * Applies std::scientific
       * @return This Puts.
       */
      const Puts& operator<<( scientific ) const {
        ss_ << std::scientific;
        return *this;
      }

      /**
       * Applies std::dec
       * @return This Puts.
       */
      const Puts& operator<<( dec ) const {
        ss_ << std::dec;
        return *this;
      }

      /**
       * Applies std::oct
       * @return This Puts.
       */
      const Puts& operator<<( oct ) const {
        ss_ << std::oct;
        return *this;
      }

      /**
       * Applies std::hex
       * @return This Puts.
       */
      const Puts& operator<<( hex ) const {
        ss_ << std::hex;
        return *this;
      }

      /**
       * Applies std::setw
       * @param w The width.
       * @return This Piuts.
       */
      const Puts& operator<<( setw w ) const {
        ss_ << std::setw( w.w_ );
        return *this;
      }

      /**
       * Applies std::setprecision
       * @param p The precision.
       * @return This Piuts.
       */
      const Puts& operator<<( setprecision p ) const {
        ss_ << std::setprecision( p.p_ );
        return *this;
      }

      /**
       * Implicit Puts conversion to string for the compiler
       * @return The string build in stringstream ss_.
       */
      operator std::string() const { return ss_.str(); };

    private:
      /**
       * Use a stringstream internally.
       */
      mutable std::stringstream ss_;

  };

}

#endif
