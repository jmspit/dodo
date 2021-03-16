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
 * @file kvstore.hpp
 * Defines the KVStore class.
 */

#ifndef dodo_kvstore_hpp
#define dodo_kvstore_hpp

#include <filesystem>
#include <list>
#include <vector>
#include <sqlite3.h>
#include <common/bytes.hpp>

/**
 * Persistent storage structures.
 */
namespace dodo::persist {

  /**
   * A persistent, multi-threaded key-value store backed by sqlite3 database (file). If each thread uses its
   * own KVStore object, no explicit synchronization is required between the threads. Cluster filesystems are
   * not supported - two processes can only write to a single SQLite file if the processes run on the same host.
   *
   * The KVStore is backed by the kvstore table. In SQLite, values in the same column can have different data types, so
   * the table can store values of any of the supported data types:
   *
   * | DataType | C++ type | SQLite type |
   * |-------------------|----------|-------------|
   * | dtInteger | int64_t | SQLITE_INTEGER |
   * | dtFloat | double | SQLITE_FLOAT |
   * | dtText | std::string | SQLITE3_TEXT |
   * | dtBlob | common::Bytes | SQLITE_BLOB |
   * | dtUnknown | used to indicate the unknown dataType of a non-existing key |  |
   *
   * In typical use the code is aware of the datatype of a key, so typical use would be
   *
   * @code
   * persist::KVStore store("mystore.db");
   * store.insertKey( "pi", 3.14 );         // insert as a double value
   * double pi = store.asDouble( "pi" );    // retrieve as a double value as we know it is a double.
   * @endcode
   *
   * Moreover, SQLite will convert (CAST) between types where possible, but revert to defaults when it can't. For example,
   *
   * @code
   * store.insertKey( "key", "value" );
   * cout <<  store.asInteger( "key" ) << endl;
   * @endcode
   * will output 0 as the string "value" does not convert to an integer. Use getMetaData to check if a key is of the expected DataType.
   *
   * It is also possible to store dodo::common::Bytes (dtBlob / SQLITE_BLOB) types.
   *
   * The SQLite database is initailized in WAL mode for performance. Make sure to use transactions to group
   * bulk insertKey or setKey as that is much faster.
   *
   * The examples/kvstore/kvstore.cpp is a simple speed test using a KVStore:
   * @include examples/kvstore/kvstore.cpp
   */
  class KVStore {
    public:

      /**
       * Meta data concerning the key.
       */
      struct MetaData {
        /** unix timestamp in UTC (seconds) */
        double last_modified = 0;
        /** number of times the value was updates (is 0 after insertKey) */
        int64_t update_count = 0;
        /** The DataType of the key's value. */
        sqlite::Query::DataType type;
      };

      /**
       * Create the KVStore object against the path. If the KVStore does not exist yet, it is
       * created. If it already exists, it is opened.
       * @param path The filesystem path of the KVStore file.
       */
      KVStore( const std::filesystem::path &path );

      /**
       * Destructor, cleanup sync and close the SQLLite database.
       */
      ~KVStore();

      /**
       * Sync all to disk - issue a SQLite full checkpoint.
       */
      void checkpoint();

      /**
       * Commit a transaction. Throws a dodo::common::Exception when no transaction has started.
       */
      void commitTransaction();

      /**
       * Delete the key or return false if the key does not exist.
       * @param key The key.
       * @return True if the key existed.
       */
      bool deleteKey( const std::string &key );

      /**
       * If the key does not exist, create it with the default and return the default.
       * If the key exists, return its value (which may not be the default).
       * @param key The key.
       * @param def The string value for the key if it does not exist.
       * @return The key value.
       */
      std::string ensureWithDefault( const std::string &key, const std::string &def );

      /**
       * If the key does not exist, create it with the default and return the default.
       * If the key exists, return its value (which may not be the default).
       * @param key The key.
       * @param def The double value for the key if it does not exist.
       * @return The key value.
       */
      double ensureWithDefault( const std::string &key, double &def );

      /**
       * If the key does not exist, create it with the default and return the default.
       * If the key exists, return its value (which may not be the default).
       * @param key The key.
       * @param def The int64_t value for the key if it does not exist.
       * @return The key value.
       */
      int64_t ensureWithDefault( const std::string &key, int64_t &def );

      /**
       * Check if the key exists. Unless the value is not required , it is more efficient
       * to get the value in one go by calling the getString(), getDouble(), getInt64() and getData()
       * members, which return false if the key does not exist.
       * @param key The key to check for.
       * @return False if the key does not exist.
       */
      bool exists( const std::string &key ) const;

      /**
       * Return a list of keys that match the filter.
       * @param keys The list that receives the keys. The list is cleared before assigning keys so it may turn up empty if
       * the filter matches no keys.
       * @param filter The SQL-style case-sensitive filter as in '%match%', 'match%'
       */
      void filterKeys( std::list<std::string>& keys, const std::string &filter );

      /**
       * Presumes the key exists, and throws an common::Exception if it does not. Otherwise, return the value
       * for the key as a double.
       * @param key
       * @return The double value for the key.
       */
      double getDouble( const std::string &key );

      /**
       * Get MetaData for the key.
       * @param key The key.
       * @return the MetaData for the key.
       */
      MetaData getMetaData( const std::string &key );

      /**
       * If the key exists, returns true and sets the value parameter.
       * @param key The key to get the value for.
       * @param value The destination value to set.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, std::string &value ) const;

      /**
       * Presumes the key exists, and throws an common::Exception if it does not. Otherwise, return the value
       * for the key as a string.
       * @param key
       * @return The string value for the key.
       */
      std::string getString( const std::string &key );

      /**
       * If the key exists, returns true and sets the value parameter.
       * @param key The key to get the value for.
       * @param value The destination value to set.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, double &value ) const;

      /**
       * If the key exists, returns true and sets the value parameter.
       * @param key The key to get the value for.
       * @param value The destination value to set.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, int64_t &value ) const;

      /**
       * If the key exists, returns true and copies (overwrites) data to the Bytes.
       * @param key The key to get the value for.
       * @param value The Bytes that will receive the data.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, common::Bytes &value ) const;

      /**
       * Insert a (key, string) pair.
       * @param key The key.
       * @param value The string value to set.
       * @return True if the key was created, false if the key already exists.
       */
      bool insertKey( const std::string &key, const std::string &value );

      /**
       * Insert a (key, double) pair.
       * @param key The key.
       * @param value The double value to set.
       * @return True if the key was created, false if the key already exists.
       */
      bool insertKey( const std::string &key, const double &value );

      /**
       * Insert a (key, int64_t) pair.
       * @param key The key.
       * @param value The int64_t value to set.
       * @return True if the key was created, false if the key already exists.
       */
      bool insertKey( const std::string &key, const int64_t &value );

      /**
       * Insert a (key, Bytes) pair. Note that the size of the data cannot exceed INT_MAX, and
       * exception is thrown if the Bytes is larger.
       * @param key The key.
       * @param oa The Bytes to insert.
       * @return True if the key was created, false if the key already exists.
       */
      bool insertKey( const std::string &key, const common::Bytes &value );

      /**
       * Optimzime, preferably called after workload and implicitly called by the destructor.
       */
      void optimize();

      /**
       * Rollback a transaction. Throws a dodo::common::Exception when no transaction has started.
       */
      void rollbackTransaction();

      /**
       * Set the string value of an existing key. The function returns false if the key does not exist.
       * @param key The key.
       * @param value The string value to set.
       * @return True if the key exists, in which case it is also updated.
       */
      bool setKey( const std::string &key, const std::string &value );

      /**
       * Set the double value of an existing key. The function returns false if the key does not exist.
       * @param key The key.
       * @param value The double value to set.
       * @return True if the key exists, in which case it is also updated.
       */
      bool setKey( const std::string &key, const double &value );

      /**
       * Set the int64_t value of an existing key. The function returns false if the key does not exist.
       * @param key The key.
       * @param value The int64_t value to set.
       * @return True if the key exists, in which case it is also updated.
       */
      bool setKey( const std::string &key, const int64_t &value );

      /**
       * Set the binary data/Bytes value of an existing key. The function returns false if the key does not exist.
       * @param key The key.
       * @param value The Bytes to set.
       * @return True if the key exists, in which case it is also updated.
       */
      bool setKey( const std::string &key, const common::Bytes &value );

      /**
       * Start a transaction. If insertKey or setKey calls are not inside a started transaction, each will commit
       * automatically and indvidually, which is mach (much!) slower for bulk operations.
       */
      void startTransaction();

      /**
       * Vaccum - clean and defragment, which may increase efficiency after heavy deletion and/or modification.
       */
      void vacuum();

    protected:

      /**
       * Create the SQLite schema.
       */
      void createSchema();

      /**
       * Prepare all SQL statements.
       */
      void prepareSQL();

      /** The filesystem path to the kvstore. */
      std::filesystem::path path_;

      sqlite::Database* db_;

      sqlite::Query* stmt_exists_;

      /** Get-value-for-key statement handle. */
      sqlite::Query* stmt_getvalue_;

      /** Insert key pair statement handle. */
      sqlite::DML* stmt_insert_;

      /** Delete key pair statement handle. */
      sqlite::DML* stmt_delete_;

      /** Update key pair statement handle. */
      sqlite::DML* stmt_update_;

      /** Key filter statement handle. */
      sqlite::Query* stmt_key_filter_;

      /** Key + value filter statement handle. */
      sqlite::Query* stmt_key_value_filter_;

      /** Get metadata statement handle. */
      sqlite::Query* stmt_metadata_;
  };

}

#endif