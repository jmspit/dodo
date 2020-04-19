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
#include <sstream>
#include "buildenv.hpp"
#include <sys/time.h>


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
       * Construct a StopWatch.
       */
      StopWatch() {
        stop_ = std::chrono::high_resolution_clock::now() - std::chrono::hours(24);
        start_ = std::chrono::high_resolution_clock::now();
      };

      /**
       * Start the stopwatch. If calling start() is omitted,
       * start_ is set to thie time StopWatch::StopWatch() was called.
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
       * - StopWatch cosntructor and time of this call.
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

}

#endif
