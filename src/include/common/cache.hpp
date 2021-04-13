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
 * @file cache.hpp
 * Defines the dodo::common::Cache template class.
 */

#ifndef common_cache_hpp
#define common_cache_hpp

#include <common/exception.hpp>
#include <threads/mutex.hpp>

#include <iostream>


namespace dodo::common {

  using namespace std::chrono_literals;

  /**
   * Simple thread-safe Cache template for arbitrary Key-Value pairs. A Cache is applicable where the get( const Key& key, Value &value )
   * that finds the key in the cache, is much faster than the load( const Key& key, Value &value ).
   * Each CacheEntry tracks the latest hit and load times. In case of pressure on the max_size, entries with the lowest hit_time
   * are evicted to make room for the new load(). Additionally, if life_time is non-zero and the Value's load time is older
   * then that, load() is invoked even if get() is a cache hit, updating both the load time and the hit time.
   *
   * The Key class must have equality (==) and ordering (<) operators for the template instantiation to compile.
   *
   * @tparam Key The unique Key type to the cached entries.
   * @tparam Value the cached Value type.
   *
   * The number of 'loads' is getMisses() + getExpiries().
   *
   * The time complexity of the get call is O(N log N) where N is the number of cache entries. The cache and lrumap are backed by
   * std::map and std::multimap respectively.
   *
   * Note that get() receives a copy of the Value in the Cache. If thread A receives Value v1, thread B may cause the Value
   * to get reloaded if its life_time expired , and that Value v2 might differ from v1. So the Cache does not guarentee that two values
   * retrurned by get on the same Key are equal no matter how close they are in wallclock time.
   *
   * @include cache.cpp
   *
   */
  template <class Key, class Value> class Cache {

    protected:

      /**
       * A CachedEntry holds the Value as well as last load and last hit time.
       */
      struct CacheEntry {
        /** The last time load was called on this Key */
        std::chrono::steady_clock::time_point load_time;
        /** The last time get or load was called on this Key. */
        std::chrono::steady_clock::time_point hit_time;
        /** The cached Value */
        Value value;
      };

      /** typedef for the Cache map */
      typedef std::map<Key,CacheEntry> CacheMap;
      /** typedef for an iterator into the Cache map */
      typedef typename CacheMap::iterator ICacheMap;
      /** typedef for a const iterator into the Cache map */
      typedef typename CacheMap::const_iterator CICacheMap;
      /** typedef for the LRU map */
      typedef std::multimap<std::chrono::steady_clock::time_point,Key> LRUMap;
      /** typedef for an iterator into the LRU map */
      typedef typename LRUMap::iterator ILRUMap;
      /** typedef for a const iterator into the LRU map */
      typedef typename LRUMap::const_iterator CILRUMap;

    public:

      /**
       * Construct a cache with at most max_size entries, and a maximum life_time of the cached entry.
       * @param max_size The maximum number of entries in the Cache.
       * @param life_time The maximum number of seconds a cached entry may live before a get hit will issue a load.  A life_time
       * of 0s will disable the aging of cached entries.
       */
      Cache( size_t max_size, std::chrono::seconds life_time ) {
        max_size_ = max_size;
        life_time_s_ = life_time;
      }

      /**
       * Wipes all cached entries.
       */
      void clear() {
        threads::Mutexer lock( mutex_ );
        cache_.clear();
        lrumap_.clear();
      }


      /**
       * Get a copy of the Value idenified by Key.
       * @param key The Key to get the Value for.
       * @param value Reference to the Value that will be assigned a copy of the cached entry.
       * @return false when the key is not loadable (load returns false).
       * @see load( const Key& key )
       */
      bool get( const Key& key, Value &value ) {
        threads::Mutexer lock( mutex_ );
        ICacheMap i = cache_.find( key );
        if ( i != cache_.end() ) {
          ++hits_;
          if ( life_time_s_ >  0s && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - i->second.load_time ) > life_time_s_ ) {
            if ( load( key, i->second.value ) ) {
              ++expired_;
              i->second.load_time = std::chrono::steady_clock::now();
            } else return false;
          }
          value = i->second.value;
          auto prev_hit_time = i->second.hit_time;
          i->second.hit_time = std::chrono::steady_clock::now();
          updateLRU( i, prev_hit_time );
          return true;
        } else {
          ++load_;
          if ( load( key, value ) ) {
            std::pair<Key,CacheEntry> p = std::pair<Key,CacheEntry>( key, { std::chrono::steady_clock::now(), std::chrono::steady_clock::now(), value } );
            std::pair<CICacheMap,bool> ret;
            ret = cache_.insert( p );
            addLRU( ret.first );
            if ( cache_.size() > max_size_ ) removeLRU();
            return true;
          } else return false;
        }
      }

      /**
       * Get the number of cache hits, which may include hits on expired entries - which still cause a load() call.
       * @return The number of times a Key was requested and present in the cache.
       */
      size_t getHits() const { return hits_; }

      /**
       * Get the number of cache loads - the number of times a Key was requested but not present in the cache.
       * @return The number of cache loades.
       */
      size_t getMisses() const { return load_; }

      /**
       * Get the number of times a Key was present in the cache (a hit) but expired so implictly calling a load.
       * @return The number of times a Key was present in the cache, but expired and cuasing a call to load.
       */
      size_t getExpiries() const { return expired_; }

      /**
       * Erase the key from the cache. A subsequent get on the same Key will call load. Does nothing if the Key does not
       * exist in the cache.
       * @param key The Key to erase.
       */
      void erase( const Key& key ) {
        threads::Mutexer lock( mutex_ );
        ICacheMap cache_entry = cache_.find( key );
        if ( cache_entry != cache_.end() ) {
          auto ret = lrumap_.equal_range( cache_entry->second.hit_time );
          ILRUMap lru_entry = lrumap_.end();
          for ( ILRUMap j = ret.first; j != ret.second; ++j ) {
            if ( key == j->second ) {
              lru_entry = j;
              break;
            }
          }
          if ( lru_entry != lrumap_.end() ) {
            lrumap_.erase(lru_entry);
          }
          cache_.erase(cache_entry);
        }
      }

      /**
       * Set the maximum size / number of entries to cache.
       * @param max_size The maximum size to set.
       */
      void setMaxSize( size_t max_size ) { threads::Mutexer lock(mutex_); max_size_ = max_size;};

      /**
       * Set the maximum life time, in seconds, of a cached entry.
       * @param max_size The maximum size to set.
       */
      void setMaxLifeTime( const std::chrono::seconds &seconds ) { threads::Mutexer lock(mutex_); life_time_s_ = seconds;};

    protected:

      /**
       * Implement to laod a value from the slow source. If the key is not loadable, return false.
       * @param key The Key to load.
       * @param value To receive the value.
       * @return False if the key could not be loaded.
       */
      virtual bool load( const Key& key, Value &value ) = 0;

      /**
       * The Cache map.
       */
      CacheMap cache_;

    private:

      /**
       * Add an entry to the lrumap_.
       * @param iter An iterator to the CacheEntry.
       */
      void addLRU( const CICacheMap& iter ) {
        lrumap_.insert( std::pair<std::chrono::steady_clock::time_point,Key>( (*iter).second.hit_time, (*iter).first ) );
      }

      /**
       * Remove the oldest CachedEntry from cache_ and lrumap_.
       */
      void removeLRU() {
        auto oldest = lrumap_.begin();
        auto entry = cache_.find( oldest->second );
        if ( entry != cache_.end() ) cache_.erase( entry );
        lrumap_.erase( oldest );
      }

      /**
       * Update an entry in the lrumap_.
       * @param iter An iterator to the CacheEntry.
       * @param prev_hit_time The previous hit time (the current key into the lrumap_)
       */
      void updateLRU( const CICacheMap& iter, std::chrono::steady_clock::time_point prev_hit_time ) {
        auto ret = lrumap_.equal_range( prev_hit_time );
        ILRUMap update = lrumap_.end();
        for ( ILRUMap i = ret.first; i != ret.second; ++i ) {
          if ( i->second == iter->first ) {
            update = i;
            break;
          }
        }
        if ( update != lrumap_.end() ) {
          lrumap_.erase( update );
          addLRU( iter );
        }
      }

      /**
       * The least-recently-used map.
       */
      LRUMap lrumap_;

      /**
       * The maximum number of cache entries.
       */
      size_t max_size_;

      /**
       * The maximum life time of a cached entry.
       */
      std::chrono::seconds life_time_s_ = 300s;

      /**
       * Mutex to synchronize threads.
       */
      threads::Mutex mutex_;

      /**
       * The number of hits.
       */
      size_t hits_ = 0;

      /**
       * The number of loades.
       */
      size_t load_ = 0;

      /**
       * The number of expiries.
       */
      size_t expired_ = 0;

  };


}

#endif