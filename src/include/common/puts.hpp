/*
 * This file is part of the arca library (https://github.com/jmspit/arca).
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
 * Defines the arca::common::Puts class.
 */

#ifndef common_puts_hpp
#define common_puts_hpp

#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

namespace dodo::common {

  using namespace std;

  /**
   * Helper class to write strings in stream format, eg
   * \code
   * string s = common::Puts() << "integer: " << 3 << " double: " << common::Puts:setprecision(2) << 3.145;
   * \endcode
   * which is very convenient in, for example, throwing exceptions;
   * \code
   * throw Exception( common::Puts() << "open failed with errorcode: " << errorcode );
   * \endcode
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
        setw( int w ) : w_(w) {};
        int w_;
      };

      /**
       * Set the precision for floating point fixed format
       * @see opeartor<<( setprecision )
       */
      struct setprecision {
        setprecision( int p ) : p_(p) {};
        int p_;
      };

      /**
       * Constructor inits to fixed format for double with precision 3.
       */
      Puts() { ss_ <<  std::setprecision(3) << std::fixed; };

      /**
       * Append a STL string.
       */
      Puts& operator<<( const string& s ) {
        ss_ << s;
        return *this;
      }

      /**
       * Append a C string.
       */
      Puts& operator<<( const char* s ) {
        ss_ << s;
        return *this;
      }

      /**
       * Append a char.
       */
      Puts& operator<<( char c ) {
        ss_ << c;
        return *this;
      }

      /**
       * Append an integer.
       */
      Puts& operator<<( int i ) {
        ss_ << i;
        return *this;
      }

      /**
       * Append a long.
       */
      Puts& operator<<( long l ) {
        ss_ << l;
        return *this;
      }

      /**
       * Append a long.
       */
      Puts& operator<<( long long l ) {
        ss_ << l;
        return *this;
      }

      /**
       * Append an unsigned integer.
       */
      Puts& operator<<( unsigned int i ) {
        ss_ << i;
        return *this;
      }

      /**
       * Append an unsigned long.
       */
      Puts& operator<<( unsigned long l ) {
        ss_ << l;
        return *this;
      }

      /**
       * Append an unsigned long long.
       */
      Puts& operator<<( unsigned long long l ) {
        ss_ << l;
        return *this;
      }

      /**
       * Append a float.
       */
      Puts& operator<<( float f ) {
        ss_ << f;
        return *this;
      }

      /**
       * Append a double.
       */
      Puts& operator<<( double d ) {
        ss_ << d;
        return *this;
      }

      /**
       * Append a void*.
       */
      Puts& operator<<( void *p ) {
        ss_ << p;
        return *this;
      }

      /**
       * Append a std::thread::id.
       */
      Puts& operator<<( std::thread::id id ) {
        ss_ << id;
        return *this;
      }

      /**
       * Appends std::endl
       */
      Puts& operator<<( Puts::endl ) {
        ss_ << std::endl;
        return *this;
      }

      /**
       * Applies std::fixed
       */
      Puts& operator<<( fixed ) {
        ss_ << std::fixed;
        return *this;
      }

      /**
       * Applies std::scientific
       */
      Puts& operator<<( scientific ) {
        ss_ << std::scientific;
        return *this;
      }

      /**
       * Applies std::dec
       */
      Puts& operator<<( dec ) {
        ss_ << std::dec;
        return *this;
      }

      /**
       * Applies std::oct
       */
      Puts& operator<<( oct ) {
        ss_ << std::oct;
        return *this;
      }

      /**
       * Applies std::hex
       */
      Puts& operator<<( hex ) {
        ss_ << std::hex;
        return *this;
      }

      /**
       * Applies std::setw
       */
      Puts& operator<<( setw w ) {
        ss_ << std::setw( w.w_ );
        return *this;
      }

      /**
       * Applies std::setprecision
       */
      Puts& operator<<( setprecision p ) {
        ss_ << std::setprecision( p.p_ );
        return *this;
      }

      /**
       * Implicit Puts conversion to string for the compiler
       * @return The string build in stringstream ss_.
       */
      operator string() const { return ss_.str(); };

    private:
      /**
       * Use a stringstream internally.
       */
      stringstream ss_;

  };

}

#endif
