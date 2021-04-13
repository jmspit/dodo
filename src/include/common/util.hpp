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
#include <regex>
#include <set>
#include <sstream>
#include <vector>
#include "buildenv.hpp"
#include <sys/time.h>

#include <yaml-cpp/yaml.h>
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
       * @return the elapsed time in seconds.
       */
      double stop() { stop_ = std::chrono::high_resolution_clock::now(); return getElapsedSeconds(); }

      /**
       * Stop the stopwatch, start the Stopwatch again and return the elapsed time since previous start.
       * @return the elapsed time in seconds.
       */
      double restart() { double t = stop(); start(); return t; };

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

    /**
     * Read the file as a single string.
     * @throw Oops when the file cannot be read.
     * @param filename the file to read from.
     * @return the file contents as a string.
     */
  std::string fileReadString( const std::string &filename );

    /**
     * Read the file as vector of strings.
     * @throw Oops when the file cannot be read.
     * @param filename the file to read from.
     * @return the file as a vector of strings
     */
  std::vector<std::string> fileReadStrings( const std::string &filename );

    /**
     * Read the file as vector of strings, return only regexp matches
     * @throw Oops when the file cannot be read.
     * @param filename the file to read from.
     * @param exp the regex that must match.
     * @return the file as a vector of strings.
     */
  std::vector<std::string> fileReadStrings( const std::string &filename, const std::regex& exp );

  /**
   * Return true when the file exists and the calling user has read access
   * @param path The path to the file.
   * @return True when the file exists and can be read.
   */
  bool fileReadAccess( const std::string& path );

  /**
   * Return true when the directory exists
   * @param path The path to the directory.
   * @return True when the directory exists.
   */
  bool directoryExists( const std::string &path );

  /**
   * Return true when the directory exists and is writable to the caller.
   * @param path The path to the directory.
   * @return True when the directory exists and is writable to the caller.
   */
  bool directoryWritable( const std::string &path );

  /**
   * Return true when the free space could be determined, and set in avail.
   * @param path The path on which to check.
   * @param avail The available free space.
   * @return False if the free space could not be determined (for example, when the file does not exist).
   */
  bool availableFileSpace( const std::string &path, size_t &avail );

  /**
   * Return the size of the file.
   * If the file does not exist or is otherwise inaccessible, returns 0.
   * @param path The path of the file.
   * @return The file size
   */
  size_t getFileSize( const std::string &path );

  /**
   * Return a datetime string in UTC (2020-07-01T20:14:36.442929Z)
   * @param tv The tv time struct.
   * @return The formatted datetime.
   */
  std::string formatDateTimeUTC( const struct timeval &tv );

  /**
   * Escape a JSOn string.
   * @param s The JSON string to escape.
   * @return The escaped string.
   */
  std::string escapeJSON( const std::string &s );

  /**
   * Template function to check existence and read YAML values of arbitrary type. The key-value
   * pair must must exist under node.
   * @param node The YAML::Node containing the key-value pair
   * @param key The YAML::Node name.
   * @return the value of type T
   * @throw common::Exception when the node does not exist.
   */
  template <typename T> T YAML_read_key( const YAML::Node &node, const std::string& key );

  /**
   * Template function to check existence and read YAML values of arbitrary type. The key-value
   * pair must must exist under node.
   * @param node The YAML::Node containing the key-value pair
   * @param key The YAML::Node name.
   * @param default_value The default value to assign when the key is missing
   * @return the value of type T
   */
  template <typename T> T YAML_read_key_default( const YAML::Node &node,
                                                 const std::string& key,
                                                 const T& default_value );

}

#endif
