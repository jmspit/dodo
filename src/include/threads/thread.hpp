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
 * @file thread.hpp
 * Defines the dodo::threads::Thread class.
 */

#ifndef threads_thread_hpp
#define threads_thread_hpp

#include <mutex>
#include <thread>
#include <string>

#include <sys/time.h>
#include <sys/resource.h>

namespace dodo::threads {

  /**
   * Abstract Thread class. Inherit and implement pure virtual run() to run threaded code.
   */
  class Thread {
    public:

      /**
       * Constructor
       */
      Thread();

      /**
       * Destructor
       */
      virtual ~Thread();

      /**
       * Start the thread.
       */
      void start();

      /**
       * Wait for the thread to join the current thread / wait for the thread to finish.
       */
      void wait();

      /**
       * Get the thread id.
       * @return that id.
       */
      std::thread::id getId() const;

      /**
       * Take a snapshot of the thread's resource usage.
       */
      void snapRUsage();

      /**
       * Get the average user mode cpu (cpu seconds/second) since thread start()
       * @return that utilization.
       */
      double getAvgUserCPU();

      /**
       * Get the average system mode cpu (cpu seconds/second) since thread start()
       * @return that utilization.
       */
      double getAvgSysCPU();

      /**
       * Get the average minor fault rate since thread start()
       * @return that rate.
       */
      double getAvgMinFltRate();

      /**
       * Get the average major fault rate since thread start()
       * @return that rate.
       */
      double getAvgMajFltRate();

      /**
       * Get the average block in rate since thread start()
       * @return that rate.
       */
      double getAvgBlkInRate();

      /**
       * Get the average block out rate since thread start()
       * @return that rate.
       */
      double getAvgBlkOutRate();

      /**
       * Get the average voluntary context switch rate since thread start()
       * @return that rate.
       */
      double getAvgVCtx();

      /**
       * Get the average involuntary context switch rate since thread start()
       * @return that rate.
       */
      double getAvgICtx();

      /**
       * Get the maximum resident set size seen on the thread.
       * @return that size.
       */
      long   getMaxRSS();

      /**
       * Get the user mode cpu (cpu seconds/second) since last sample.
       * @return that utilization.
       */
      double getLastUserCPU();

      /**
       * Get the system mode cpu (cpu seconds/second) since last sample.
       * @return that utilization.
       */
      double getLastSysCPU();

      /**
       * Get the minor fault rate since last sample.
       * @return that rate.
       */
      double getLastMinFltRate();

      /**
       * Get the major fault rate since last sample.
       * @return that rate.
       */
      double getLastMajFltRate();

      /**
       * Get last block in rate since last sample.
       * @return that rate.
       */
      double getLastBlkInRate();

      /**
       * Get last block out rate since last sample.
       * @return that rate.
       */
      double getLastBlkOutRate();

      /**
       * Get voluntary context switch rate since last sample.
       * @return that rate.
       */
      double getLastVCtx();

      /**
       * Get involuntary context switch rate since last sample.
       * @return that rate.
       */
      double getLastICtx();

      /**
       * Time, in seconds, since thread start.
       * @return that time.
       */
      double getRunTime();

      /**
       * Time, in seconds, since last statistic update
       * @return that time.
       */
      double getSnapDiffTime();

    protected:

      /**
       * Decsendants must override the run function.
       */
      virtual void run() = 0;

      /**
       * The std::tread object.
       */
      std::thread*  thread_;

      /**
       * Time Thread started.
       */
      struct timeval start_time_;

      /**
       * Time of previous statistics snapshot
       */
      struct timeval prev_snap_time_;

      /**
       * Time of last statistics snapshot
       */
      struct timeval snap_time_;

      /**
       * Previous statistics
       */
      struct rusage prev_rusage_;

      /**
       * Last statistics
       */
      struct rusage rusage_;

    private:

      /**
       * Posix thread method - calls dodo::threads::Thread:::run().
       * @param context Is used to pass a Thread* so that its run() method can be called.
       */
      static void* thread_method( void* context );
  };

}

#endif
