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
 * @file mutex.hpp
 * Defines the dodo::threads::Mutex and dodo::threads::Mutexer classes.
 */

#ifndef threads_mutex_hpp
#define threads_mutex_hpp

#include <mutex>
#include <string>

namespace dodo::threads {

  using namespace std;

  /**
   * A Mutex to synchronize access to resources between threads.
   *
   * Use a Mutexer to control a Mutex by mere scope.
   *
   * @see Mutexer
   * @see std::mutex
   */
  class Mutex {
    public:

      /**
       * Construct a Mutex.
       */
      Mutex() : mutex_() {};

      /**
       * Destruct a Mutex.
       */
      virtual ~Mutex() {};

      /**
       * Waits for a Mutex and locks it (atomically). You must not call lock if the lock is already
       * held by the calling thread, that is a deadlock.
       * @see std::mutex::lock()
       */
      void lock() { mutex_.lock(); };

      /**
       * If the Mutex is currently not locked, lock it and return true. If the Mutex is locked, return false.
       * You must not call lock if the lock is alread held by the calling thread, that is a deadlock.
       * Mutex from the same thread twice - that is a deadlock.
       * @return true if the Mutex was locked. false if the Mutex was already locked.
       * @see std::mutex::try_lock()
       */
      bool tryLock() { return mutex_.try_lock(); };

      /**
       * Unlocks the Mutex. Calling unLOck whilst the calling thread is not holding the lock causes undefined
       * behavior in the Mutex.
       * @see std::mutex::unlock()
       */
      void unLock() { mutex_.unlock(); };

    private:
      /**
       * the internal std::mutex
       */
      std::mutex mutex_;
  };

  /**
   * Waits for and locks the Mutex on construction, unlocks the Mutex when this Mutexer is destructed.

   * In the below code, it would be safe to call addInt from multiple threads. The Mutexer object calls
   * thelist_mutex.lock() when constructed, and thelist_mutex.unLock() when it goes out of
   * scope( the function returns or an exception is thrown).
   * @code
   * // Mutex to protect thelist
   * threads::Mutex thelist_mutex;
   * std::list<int> thelist;
   *
   * function addInt( int i ) {
   *   Mutexer lock( thelist_mutex );
   *   thelist.push_back( i );
   * }
   * @endcode
   * @see Mutex
   */
  class Mutexer {
    public:
      /**
       * Constructor.
       * @param mutex The mutex to guard.
       */
      Mutexer( Mutex& mutex ) : mutex_(mutex) { mutex_.lock(); };

      /**
       * Destructor.
       * Unlocks the guarded mutex.
       */
      ~Mutexer() { mutex_.unLock(); };

    private:

      /**
       * Reference to the guarded mutex.
       */
      Mutex &mutex_;
  };

}

#endif
