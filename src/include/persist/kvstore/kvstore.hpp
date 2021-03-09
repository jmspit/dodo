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
#include <common/octetarray.hpp>

/**
 * Persistent storage structures.
 */
namespace dodo::persist {

  /**
   * A persistent, multi-threaded key-value store backed by sqlite3 storage (file). The same file
   * can safely be used by multiple processes (but not across nodes). If each thread uses its
   * own KVStore object, no explicit synchronization is required.
   *
   * The KVStore is backed by a SQLite database with a table kvstore( key text primary key , value not null ). In SQLite,
   * column values can have different data types, so the single table can store any of the supported data types
   *
   * | DataType | C++ type | SQLite type |
   * |-------------------|----------|-------------|
   * | dtInteger | int64_t | SQLITE_INTEGER |
   * | dtFloat | double | SQLITE_FLOAT |
   * | dtText | std::string | SQLITE3_TEXT |
   * | dtBlob | common::OctetArray | SQLITE_BLOB |
   * | dtUnknown | used to indicate the unknown dataType of a non-existing key |  |
   *
   * In typical use the caller is aware of the datatype of a key, so typical use would be
   *
   * @code
   * persist::KVStore store("mystore.db");
   * store.insertKey( "pi", 3.14 );         // insert as a double value
   * double pi = store.asDouble( "pi" );    // retrieve as a double value
   * @endcode
   *
   * Moreover, SQLite will convert (CAST) between types where possible, but revert to defaults when it can't. For example,
   *
   * @code
   * store.insertKey( "key", "value" );
   * cout <<  store.asInteger( "key" ) << endl;
   * @endcode
   * will output 0 as the string "value" does not convert to an integer.
   */
  class KVStore {
    public:

      enum DataType {
        dtInteger = SQLITE_INTEGER, /**< Integer type (int64_t) */
        dtFloat   = SQLITE_FLOAT,   /**< Floating point type (double) */
        dtText    = SQLITE3_TEXT,   /**< Text type (string) */
        dtBlob    = SQLITE_BLOB,    /**< Binary data type (OctetArray) */
        dtNull    = SQLITE_NULL,    /**< NULL type (unset and undefined) */
        dtUnknown = 91,             /**< Used to indicate a DataType that could not be determined */
      };

      /**
       * Create the KVStore object against the path. If the KVStore does not exist yet, it is
       * created. If it already exists, it is opened.
       * @param path The filesystem path of the KVStore file.
       */
      KVStore( const std::filesystem::path &path );

      ~KVStore();

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
       * Insert a (key, binary data) pair.
       * @param key The key.
       * @param data A pointer to the data.
       * @param size The size of the data.
       * @return True if the key was created, false if the key already exists.
       */
      bool insertKey( const std::string &key, const void* data, int size );

      /**
       * Insert a (key, OctetArray) pair. Note that the size of the data cannot exceed INT_MAX, and
       * exception is thrown if the OctetArray is larger.
       * @param key The key.
       * @param oa The OctetArray to insert.
       * @return True if the key was created, false if the key already exists.
       */
      bool insertKey( const std::string &key, const common::OctetArray &oa );

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
       * Set the binary data value of an existing key. The function returns false if the key does not exist.
       * @param key The key.
       * @param data The data to set.
       * @param size The size of the data to set.
       * @return True if the key exists, in which case it is also updated.
       */
      bool setKey( const std::string &key, const void* data, int size );

      /**
       * Set the binary data/OctetArray value of an existing key. The function returns false if the key does not exist.
       * @param key The key.
       * @param oa The OctetArray to set.
       * @param size The size of the data to set.
       * @return True if the key exists, in which case it is also updated.
       */
      bool setKey( const std::string &key, const common::OctetArray &oa );

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
       * Presumes the key exists, and throws an common::Exception if it does not. Otherwise, return the value
       * for the key as a string.
       * @param key
       * @return The string value for the key.
       */
      std::string getString( const std::string &key );

      double getDouble( const std::string &key );

      /**
       * If the key exists, returns true and sets the value parameter.
       * @param key The key to get the value for.
       * @param value The destination value to set.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, std::string &value ) const;

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
       * If the key exists, returns true and sets the data and size parameter. The blob is copied into
       * memory allocated at the pointer returned in data, memory which becomes the caller's responsibility
       * to clean up.
       * @param key The key to get the value for.
       * @param data The pointer that will be set and points to the data.
       * @param size The size of the data.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, const void* &data, int &size ) const;

      /**
       * If the key exists, returns true and copies (overwrites) data to the OctetArray.
       * @param key The key to get the value for.
       * @param data The OctetArray that will receive the data.
       * @return False if the key does not exist.
       */
      bool getValue( const std::string &key, common::OctetArray &data ) const;

      /**
       * Check if the key exists. Unless the value is not required , it is more efficient
       * to get the value in one go by calling the getString(), getDouble(), getInt64() and getData()
       * members, which return false if the key does not exist.
       * @param key The key to check for.
       * @return False if the key does not exist.
       */
      bool exists( const std::string &key ) const;

      DataType getDataType( const std::string &key ) const;

      /**
       * Delete the key or return false if the key does not exist.
       * @param key The key.
       * @return True if the key existed.
       */
      bool deleteKey( const std::string &key );

      /**
       * Return a list of keys that match the filter.
       * @param keys The list that receives the keys. The list is cleared before assigning keys so it may turn up empty if
       * the filter matches no keys.
       * @param filter The SQL-style case-sensitive filter as in '%match%', 'match%'
       */
      void filterKeys( std::list<std::string>& keys, const std::string &filter );

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

      /** The database handle. */
      sqlite3 *database_;

      /** Existence-check statement handle. */
      sqlite3_stmt* stmt_exists_;

      /** Get-value-for-key statement handle. */
      sqlite3_stmt* stmt_getvalue_;

      /** Insert key pair statement handle. */
      sqlite3_stmt* stmt_insert_;

      /** Delete key pair statement handle. */
      sqlite3_stmt* stmt_delete_;

      /** Update key pair statement handle. */
      sqlite3_stmt* stmt_update_;

      /** Key filter statement handle. */
      sqlite3_stmt* stmt_key_filter_;

      /** Key + value filter statement handle. */
      sqlite3_stmt* stmt_key_value_filter_;
  };

}

#endif