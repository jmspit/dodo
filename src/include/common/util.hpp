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
 * @file util.hpp
 * Defines utilities.
 */

#ifndef common_util_hpp
#define common_util_hpp

#include <chrono>
#include <string>
#include <ostream>
#include <set>
#include <sstream>
#include <vector>
#include "buildenv.hpp"
#include <sys/time.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace dodo::common {

  void dumpBinaryData( std::ostream &out, const std::string &s, size_t width );

  /**
   * StopWatch timing class.
   * @code
   * StopWatch sw;
   * sw.start();
   * ...
   * sw.stop();
   * double seconds = sw.getElapsedSecond();
   * @endcode
   */
  class StopWatch {
    public:

      /**
       * Construct a StopWatch. The start time is taken here, or can be reset by calling start().
       */
      StopWatch() {
        stop_ = std::chrono::high_resolution_clock::now() - std::chrono::hours(24);
        start_ = std::chrono::high_resolution_clock::now();
      };

      /**
       * Start the stopwatch. If calling start() is omitted,
       * start_ is set to the time StopWatch::StopWatch() was called.
       */
      void start() {
        stop_ = std::chrono::high_resolution_clock::now() - std::chrono::hours(24);
        start_ = std::chrono::high_resolution_clock::now();
      }

      /**
       * Stop the stopwatch.
       */
      void stop() { stop_ = std::chrono::high_resolution_clock::now(); }

      /**
       * Return the number of seconds between
       * - start() and stop() calls
       * - start() and time of this call
       * - StopWatch constructor and time of this call.
       * @return The number of seconds.
       */
      double getElapsedSeconds() const {
        std::chrono::duration<double> diff;
        if ( stop_ < start_ ) {
          diff = std::chrono::high_resolution_clock::now() - start_;
        } else {
          diff = stop_ - start_;
        }
        return diff.count();
      };

    private:
      /** Start time */
      std::chrono::time_point<std::chrono::high_resolution_clock> start_;
      /** Stop time */
      std::chrono::time_point<std::chrono::high_resolution_clock> stop_;

  };


  /**
   * Read from a file, expecting it to contain a (signed) int.
   * @param file The file name.
   * @param i The int read.
   * @return False if the read or conversion failed.
   */
  bool fileReadInt( const std::string &file, int &i );

  /**
   * Return difference in seconds as a double. t2 must be > t1 for the return value to be positive.
   * @param t1 The earlier time.
   * @param t2 The later time.
   * @return the difference in seconds from t1 to t2.
   */
  inline double getSecondDiff( struct timeval& t1, struct timeval &t2 ) {
    return (double)t2.tv_sec - (double)t1.tv_sec + ( (double)t2.tv_usec - (double)t1.tv_usec ) / 1.0E6;
  }

  /**
   * Split a string into substrings.
   * @param src The string to split from.
   * @param delimiter The delimiter to use.
   * @return A vector of strings.
   */
  inline std::vector<std::string> split( const std::string &src, char delimiter = ' ' ) {
    std::vector<std::string> result;
    std::istringstream is(src);
    std::string s;
    while ( getline( is, s, delimiter ) ) {
      result.push_back( s );
    }
    return result;
  }

  /**
   * Split a string into substrings by delimiter - unless the delimiter is escaped.
   * Passing ( 'one:two$:three', '$', ':' ) splits into ['one','two:three'].
   * Passing ( 'one:t$$wo:three', '$', ':' ) splits into ['one','t$$wo','three'].
   * So the escape character only has effect when followed by a delimiter, otherwise
   * taken as input.
   * @param src The string to split from.
   * @param escape The escape characters to use
   * @param delimiter The delimiter to use.
   * @return A vector of strings.
   */
  inline std::vector<std::string> escapedSplit( const std::string &src, std::set<char> escape, char delimiter = ' ' ) {
    enum State { stInit, stEscape, stInput };
    std::vector<std::string> result;
    std::stringstream tmp;
    State state = stInit;
    for ( auto c : src ) {
      if ( escape.find(c) != escape.end() ) {
        state = stEscape;
      } else if ( c == delimiter ) {
        if ( state != stEscape ) {
          result.push_back( tmp.str() );
          tmp.str("");
          state = stInput;
        } else {
          tmp << c;
          state = stInput;
        }
      } else {
        tmp << c;
      }
    }
    if ( tmp.str().length() > 0 ) result.push_back( tmp.str() );
    return result;
  }

  /**
   * Convert the data contents of an OpenSSL BIO to a std::string.
   * @param bio The source BIO.
   * @return The string representation of the BIO contents.
   */
  std::string bio2String( BIO* bio );

  /**
   * Write OpenSSL errors occurred in this thread to ostream, and clear their error state.
   * @param out The std::ostream to write to.
   * @param terminator The char to use to separate lines.
   * @return The number of SSL errors written.
   */
  size_t writeSSLErrors( std::ostream& out, char terminator );

  /**
   * Get all OpenSSL errors as a single string, and clear their error state.
   * @param terminator The terminator character for a single error line. If 0, no character will be appended.
   * @return The string.
   * @see writeSSLErrors( ostream& out)
   */
  std::string getSSLErrors( char terminator );

}

#endif
