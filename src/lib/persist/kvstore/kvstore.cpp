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
 * @file kvstore.cpp
 * Implements the dodo::persistent::KVStore class..
 */


#include <persist/kvstore/kvstore.hpp>
#include <common/exception.hpp>
#include <common/util.hpp>

#include <iostream>
#include <climits>

namespace dodo::persist {

  KVStore::KVStore( const std::filesystem::path &path ) {
    path_ = path;
    database_ = nullptr;
    stmt_exists_ = nullptr;
    stmt_getvalue_ = nullptr;
    stmt_insert_ = nullptr;
    stmt_delete_ = nullptr;
    stmt_update_ = nullptr;
    stmt_key_filter_ = nullptr;
    stmt_key_value_filter_ = nullptr;
    auto rc = sqlite3_open_v2( path.c_str(), &database_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    createSchema();
    prepareSQL();
  }

  KVStore::~KVStore() {
    optimize();
    checkPoint();
    if ( stmt_key_value_filter_ ) sqlite3_finalize( stmt_key_value_filter_ );
    if ( stmt_key_filter_ ) sqlite3_finalize( stmt_key_filter_ );
    if ( stmt_update_ ) sqlite3_finalize( stmt_update_ );
    if ( stmt_delete_ ) sqlite3_finalize( stmt_delete_ );
    if ( stmt_insert_ ) sqlite3_finalize( stmt_insert_ );
    if ( stmt_getvalue_ ) sqlite3_finalize( stmt_getvalue_ );
    if ( stmt_exists_ ) sqlite3_finalize( stmt_exists_ );
    if ( database_ ) sqlite3_close( database_ );
  }

  void KVStore::createSchema() {
    std::string sql = "PRAGMA journal_mode=WAL;"
                      "CREATE TABLE IF NOT EXISTS kvstore ( "
                      "key TEXT NOT NULL PRIMARY KEY, "
                      "value NOT NULL )";
    auto rc = sqlite3_exec( database_, sql.c_str(), nullptr, 0, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
  }

  void KVStore::checkPoint() {
    auto rc =  sqlite3_wal_checkpoint_v2( database_, nullptr, SQLITE_CHECKPOINT_FULL, nullptr, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
  }

  void KVStore::optimize() {
    std::string sql = "PRAGMA optimize;";
    auto rc = sqlite3_exec( database_, sql.c_str(), nullptr, 0, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
  }

  void KVStore::vacuum() {
    std::string sql = "VACUUM;";
    auto rc = sqlite3_exec( database_, sql.c_str(), nullptr, 0, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
  }

  void KVStore::prepareSQL() {
    std::string sql = "PRAGMA case_sensitive_like=ON;PRAGMA journal_mode=WAL;";
    auto rc = sqlite3_exec( database_, sql.c_str(), nullptr, 0, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "SELECT COUNT(1) FROM kvstore WHERE key = ?";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_exists_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "SELECT value FROM kvstore WHERE key = ?";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_getvalue_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "INSERT INTO kvstore ( key, value ) VALUES ( ?, ? )";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_insert_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "DELETE FROM kvstore WHERE key = ?";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_delete_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "UPDATE kvstore SET value = ? WHERE key = ?";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_update_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "SELECT key FROM kvstore WHERE key LIKE ? ORDER BY key";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_key_filter_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );

    sql = "SELECT key, value FROM kvstore WHERE key LIKE ? ORDER BY key";
    rc = sqlite3_prepare_v2( database_, sql.c_str(), static_cast<int>(sql.size()), &stmt_key_value_filter_, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
  }

  std::string KVStore::ensureWithDefault( const std::string &key, const std::string &def ) {
    std::string result;
    if ( getValue( key, result ) ) {
      return result;
    } else {
      insertKey( key, def );
      return def;
    }
  }

  double KVStore::ensureWithDefault( const std::string &key, double &def ) {
    double result;
    if ( getValue( key, result ) ) {
      return result;
    } else {
      insertKey( key, def );
      return def;
    }
  }

  int64_t KVStore::ensureWithDefault( const std::string &key, int64_t &def ) {
    int64_t result;
    if ( getValue( key, result ) ) {
      return result;
    } else {
      insertKey( key, def );
      return def;
    }
  }

  KVStore::DataType KVStore::getDataType( const std::string &key ) const {
    DataType dt = dtUnknown;
    auto rc = sqlite3_bind_text( stmt_getvalue_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_getvalue_ );
    if ( rc == SQLITE_DONE ) dt = dtUnknown;
    else if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    else dt = static_cast<DataType>( sqlite3_column_type( stmt_getvalue_, 0 ) );
    rc = sqlite3_clear_bindings( stmt_getvalue_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_getvalue_ );
    return dt;
  }

  bool KVStore::exists( const std::string &key ) const {
    auto rc = sqlite3_bind_text( stmt_exists_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_exists_ );
    if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    auto count = sqlite3_column_int( stmt_exists_, 0 );
    rc = sqlite3_clear_bindings( stmt_exists_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_exists_ );
    return count == 1;
  }

  bool KVStore::getValue( const std::string &key, std::string &value ) const {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_getvalue_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_getvalue_ );
    if ( rc == SQLITE_DONE ) result = false;
    else if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    else {
      const unsigned char *t = sqlite3_column_text( stmt_getvalue_, 0 );
      if ( t ) value = std::string( reinterpret_cast<const char*>(t) ); else value = "";
    }
    rc = sqlite3_clear_bindings( stmt_getvalue_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_getvalue_ );
    return result;
  }

  bool KVStore::getValue( const std::string &key, double &value ) const {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_getvalue_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_getvalue_ );
    if ( rc == SQLITE_DONE ) result = false;
    else if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    else value = sqlite3_column_double( stmt_getvalue_, 0 );
    rc = sqlite3_clear_bindings( stmt_getvalue_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_getvalue_ );
    return result;
  }

  bool KVStore::getValue( const std::string &key, int64_t &value ) const {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_getvalue_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_getvalue_ );
    if ( rc == SQLITE_DONE ) result = false;
    else if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    else value = sqlite3_column_int64( stmt_getvalue_, 0 );
    rc = sqlite3_clear_bindings( stmt_getvalue_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_getvalue_ );
    return result;
  }

  bool KVStore::getValue( const std::string &key, const void* &data, int &size ) const {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_getvalue_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_getvalue_ );
    if ( rc == SQLITE_DONE ) result = false;
    else if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    else {
      const void* tmp_data = sqlite3_column_blob( stmt_getvalue_, 0 );
      size = sqlite3_column_bytes( stmt_getvalue_, 0 );
      data = malloc( size );
      if ( data == nullptr ) throw_Exception( "malloc failed" );
      memcpy( const_cast<void*>(data), const_cast<void*>(tmp_data), size );
    }
    rc = sqlite3_clear_bindings( stmt_getvalue_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_getvalue_ );
    return result;
  }

  bool KVStore::getValue( const std::string &key, common::OctetArray &data ) const {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_getvalue_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_getvalue_ );
    if ( rc == SQLITE_DONE ) result = false;
    else if ( rc != SQLITE_ROW ) throw_Exception( sqlite3_errmsg(database_) );
    else {
      const common::Octet* tmp_data = static_cast<const common::Octet*>( sqlite3_column_blob( stmt_getvalue_, 0 ) );
      int size = sqlite3_column_bytes( stmt_getvalue_, 0 );
      data.free();
      data.append( tmp_data, size );
    }
    rc = sqlite3_clear_bindings( stmt_getvalue_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_getvalue_ );
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const std::string &value ) {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_insert_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_text( stmt_insert_, 2, value.c_str(), static_cast<int>(value.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_insert_ );
    if ( rc == SQLITE_CONSTRAINT ) result = false;
    else if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_clear_bindings( stmt_insert_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_insert_ );
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const double &value ) {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_insert_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_double( stmt_insert_, 2, value );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_insert_ );
    if ( rc == SQLITE_CONSTRAINT ) result = false;
    else if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_clear_bindings( stmt_insert_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_insert_ );
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const int64_t &value ) {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_insert_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_int64( stmt_insert_, 2, value );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_insert_ );
    if ( rc == SQLITE_CONSTRAINT ) result = false;
    else if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_clear_bindings( stmt_insert_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_insert_ );
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const void* data, int size ) {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_insert_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_blob( stmt_insert_, 2, data, size, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_insert_ );
    if ( rc == SQLITE_CONSTRAINT ) result = false;
    else if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_clear_bindings( stmt_insert_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_insert_ );
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const common::OctetArray &oa ) {
    bool result = true;
    auto rc = sqlite3_bind_text( stmt_insert_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    if ( oa.getSize() > INT_MAX ) throw_Exception( "OctetArray too large (INT_MAX)");
    rc = sqlite3_bind_blob( stmt_insert_, 2, oa.getArray(), static_cast<int>(oa.getSize()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_insert_ );
    if ( rc == SQLITE_CONSTRAINT ) result = false;
    else if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_clear_bindings( stmt_insert_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_insert_ );
    return result;
  }

  bool KVStore::setKey( const std::string &key, const std::string &value ) {
    auto rc = sqlite3_bind_text( stmt_update_, 1, value.c_str(), static_cast<int>(value.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_text( stmt_update_, 2, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_update_ );
    if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    int count = sqlite3_changes( database_ );
    rc = sqlite3_clear_bindings( stmt_update_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_update_ );
    return count == 1;
  }

  bool KVStore::setKey( const std::string &key, const double &value ) {
    auto rc = sqlite3_bind_double( stmt_update_, 1, value );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_text( stmt_update_, 2, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_update_ );
    if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    int count = sqlite3_changes( database_ );
    rc = sqlite3_clear_bindings( stmt_update_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_update_ );
    return count == 1;
  }

  bool KVStore::setKey( const std::string &key, const int64_t &value ) {
    auto rc = sqlite3_bind_int64( stmt_update_, 1, value );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_text( stmt_update_, 2, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_update_ );
    if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    int count = sqlite3_changes( database_ );
    rc = sqlite3_clear_bindings( stmt_update_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_update_ );
    return count == 1;
  }

  bool KVStore::setKey( const std::string &key, const void* data, int size ) {
    auto rc = sqlite3_bind_blob( stmt_update_, 1, data, size, nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_text( stmt_update_, 2, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_update_ );
    if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    int count = sqlite3_changes( database_ );
    rc = sqlite3_clear_bindings( stmt_update_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_update_ );
    return count == 1;
  }

  bool KVStore::setKey( const std::string &key, const common::OctetArray &oa ) {
    if ( oa.getSize() > INT_MAX ) throw_Exception( "OctetArray too large (INT_MAX)");
    auto rc = sqlite3_bind_blob( stmt_update_, 1, oa.getArray(), static_cast<int>(oa.getSize()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_bind_text( stmt_update_, 2, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_update_ );
    if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    int count = sqlite3_changes( database_ );
    rc = sqlite3_clear_bindings( stmt_update_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_update_ );
    return count == 1;
  }

  bool KVStore::deleteKey( const std::string &key ) {
    auto rc = sqlite3_bind_text( stmt_delete_, 1, key.c_str(), static_cast<int>(key.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_delete_ );
    if ( rc != SQLITE_DONE ) throw_Exception( sqlite3_errmsg(database_) );
    int count = sqlite3_changes( database_ );
    rc = sqlite3_clear_bindings( stmt_delete_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_delete_ );
    return count == 1;
  }

  void KVStore::filterKeys( std::list<std::string>& keys, const std::string &filter ) {
    keys.clear();
    auto rc = sqlite3_bind_text( stmt_key_filter_, 1, filter.c_str(), static_cast<int>(filter.length()), nullptr );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    rc = sqlite3_step( stmt_key_filter_ );
    while ( rc == SQLITE_ROW ) {
      const unsigned char *dbkey = sqlite3_column_text( stmt_key_filter_, 0 );
      std::string t = reinterpret_cast<const char*>( dbkey );
      keys.push_back( t );
      rc = sqlite3_step( stmt_key_filter_ );
    }
    rc = sqlite3_clear_bindings( stmt_key_filter_ );
    if ( rc != SQLITE_OK ) throw_Exception( sqlite3_errmsg(database_) );
    sqlite3_reset( stmt_key_filter_ );
  }

};