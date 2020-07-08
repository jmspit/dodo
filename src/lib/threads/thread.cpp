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
 * @file thread.cpp
 * Implements the dodo::threads::Thread class.
 */

#include <threads/thread.hpp>
#include <common/util.hpp>

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>


namespace dodo::threads {

  void* Thread::thread_method( void* context ) {
    Thread* t = ((Thread*)context);
    t->tid_ = static_cast<pid_t>(syscall(SYS_gettid));
    if ( t ) t->run();
    t->tid_  = 0;
    return 0;
  }

  Thread::Thread() : thread_(0), tid_(0) {
    gettimeofday( &start_time_, NULL );
    gettimeofday( &prev_snap_time_, NULL );
    prev_snap_time_.tv_sec -= 1;
    gettimeofday( &snap_time_, NULL );
    memset( &prev_rusage_, 0, sizeof( prev_rusage_ ) );
    memset( &rusage_, 0, sizeof( rusage_ ) );
  };


  Thread::~Thread() {
    if ( thread_ ) delete thread_;
  }

  void Thread::start() {
    if ( !thread_ ) {
      thread_ = new std::thread( this->thread_method, this );
      gettimeofday( &start_time_, NULL );
      snapRUsage();
    }
  }

  void Thread::wait() {
    if ( thread_ && thread_->joinable() ) thread_->join();
  }

  void Thread::snapRUsage() {
    prev_rusage_ = rusage_;
    prev_snap_time_ = snap_time_;
    getrusage( RUSAGE_THREAD, &rusage_ );
    gettimeofday( &snap_time_, NULL );
  }

  std::thread::id Thread::getId() const {
    if ( thread_ ) return thread_->get_id();
    return std::thread::id();
  }

  double Thread::getAvgUserCPU() {
    return ((double)rusage_.ru_utime.tv_sec + (double)rusage_.ru_utime.tv_usec/1.0E6)/getRunTime();
  }

  double Thread::getAvgSysCPU() {
    return ((double)rusage_.ru_stime.tv_sec + (double)rusage_.ru_stime.tv_usec/1.0E6)/getRunTime();
  }

  double Thread::getAvgMinFltRate() {
    return (double)rusage_.ru_minflt/getRunTime();
  }

  double Thread::getAvgMajFltRate() {
    return (double)rusage_.ru_majflt/getRunTime();
  }

  double Thread::getAvgBlkInRate() {
    return (double)rusage_.ru_inblock/getRunTime();
  }

  double Thread::getAvgBlkOutRate() {
    return (double)rusage_.ru_oublock/getRunTime();
  }

  double Thread::getAvgVCtx() {
    return (double)rusage_.ru_nvcsw/getRunTime();
  }

  double Thread::getAvgICtx() {
    return (double)rusage_.ru_nivcsw/getRunTime();
  }

  long   Thread::getMaxRSS() {
    return rusage_.ru_maxrss;
  }

  double Thread::getLastUserCPU() {
    return ( ( (double)rusage_.ru_utime.tv_sec + (double)rusage_.ru_utime.tv_usec/1.0E6 ) -
             ( (double)prev_rusage_.ru_utime.tv_sec + (double)prev_rusage_.ru_utime.tv_usec/1.0E6 ) ) / getSnapDiffTime();
  }

  double Thread::getLastSysCPU() {
    return ( ( (double)rusage_.ru_stime.tv_sec + (double)rusage_.ru_stime.tv_usec/1.0E6 ) -
             ( (double)prev_rusage_.ru_stime.tv_sec + (double)prev_rusage_.ru_stime.tv_usec/1.0E6 ) ) / getSnapDiffTime();
  }

  double Thread::getLastMinFltRate() {
    return ( ( (double)rusage_.ru_minflt ) -
             ( (double)prev_rusage_.ru_minflt ) ) / getSnapDiffTime();
  }

  double Thread::getLastMajFltRate() {
    return ( ( (double)rusage_.ru_majflt ) -
             ( (double)prev_rusage_.ru_majflt ) ) / getSnapDiffTime();
  }

  double Thread::getLastBlkInRate() {
    return ( ( (double)rusage_.ru_inblock ) -
             ( (double)prev_rusage_.ru_inblock ) ) / getSnapDiffTime();
  }

  double Thread::getLastBlkOutRate() {
    return ( ( (double)rusage_.ru_oublock ) -
             ( (double)prev_rusage_.ru_oublock ) ) / getSnapDiffTime();
  }

  double Thread::getLastVCtx() {
    return ( ( (double)rusage_.ru_nvcsw ) -
             ( (double)prev_rusage_.ru_nvcsw ) ) / getSnapDiffTime();
  }

  double Thread::getLastICtx() {
    return ( ( (double)rusage_.ru_nivcsw ) -
             ( (double)prev_rusage_.ru_nivcsw ) ) / getSnapDiffTime();
  }

  double Thread::getRunTime() {
    return common::getSecondDiff( start_time_, snap_time_ );
  }

  double Thread::getSnapDiffTime() {
    return common::getSecondDiff( prev_snap_time_, snap_time_ );
  }

}
