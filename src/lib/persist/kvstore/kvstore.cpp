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


#include <persist/sqlite/sqlite.hpp>
#include <persist/kvstore/kvstore.hpp>
#include <common/exception.hpp>
#include <common/util.hpp>

#include <iostream>
#include <climits>

namespace dodo::persist {

  KVStore::KVStore( const std::filesystem::path &path ) {
    path_ = path;
    db_ = new sqlite::Database( path_ );

    stmt_exists_ = new sqlite::Query( *db_ );
    stmt_getvalue_ = new sqlite::Query( *db_ );;
    stmt_insert_ = new sqlite::DML( *db_ );;
    stmt_delete_ = new sqlite::DML( *db_ );;
    stmt_update_ = new sqlite::DML( *db_ );;
    stmt_key_filter_ = new sqlite::Query( *db_ );;;
    stmt_key_value_filter_ = new sqlite::Query( *db_ );;;
    stmt_metadata_ = new sqlite::Query( *db_ );;;
    createSchema();
    prepareSQL();
  }

  KVStore::~KVStore() {
    if ( stmt_metadata_ ) delete stmt_metadata_;
    if ( stmt_key_value_filter_ ) delete stmt_key_value_filter_ ;
    if ( stmt_key_filter_ ) delete stmt_key_filter_;
    if ( stmt_update_ ) delete stmt_update_;
    if ( stmt_delete_ ) delete stmt_delete_;
    if ( stmt_insert_ ) delete stmt_insert_;
    if ( stmt_getvalue_ ) delete stmt_getvalue_;
    if ( stmt_exists_ ) delete stmt_exists_;
    optimize();
    checkpoint();
    if( db_) delete db_;
  }

  void KVStore::checkpoint() {
    db_->checkPointFull();
  }

  void KVStore::commitTransaction() {
    db_->commit();
  }

  void KVStore::createSchema() {
    {
      sqlite::Query pragma( *db_ );
      pragma.prepare( "PRAGMA journal_mode=WAL;" );
      pragma.step();
    }
    {
      sqlite::DDL ddl( *db_ );
      ddl.prepare(  "CREATE TABLE IF NOT EXISTS kvstore ( "
                    "key TEXT NOT NULL PRIMARY KEY, "
                    "value NOT NULL, "
                    "modified NUMBER NOT NULL DEFAULT ((julianday('now') - 2440587.5) * 86400.0), "
                    "updates INTEGER NOT NULL DEFAULT 0"
                    " )" );
      ddl.execute();
    }
  }

  bool KVStore::deleteKey( const std::string &key ) {
    stmt_delete_->bind( 1, key );
    int affected = stmt_delete_->execute();
    stmt_delete_->reset();
    return (affected == 1);
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

  bool KVStore::exists( const std::string &key ) const {
    stmt_exists_->bind( 1, key );
    stmt_exists_->step();
    int count = stmt_exists_->getInt(0);
    stmt_exists_->reset();
    return count == 1;
  }

  void KVStore::filterKeys( std::list<std::string>& keys, const std::string &filter ) const {
    keys.clear();
    stmt_key_filter_->bind( 1, filter );
    while ( stmt_key_filter_->step() ) {
      keys.push_back( stmt_key_filter_->getText( 0 ) );
    }
    stmt_key_filter_->reset();
  }

  KVStore::MetaData KVStore::getMetaData( const std::string &key ) const {
    MetaData data;
    stmt_metadata_->bind( 1, key );
    if ( stmt_metadata_->step() ) {
      data.last_modified = stmt_metadata_->getDouble( 0 );
      data.update_count = stmt_metadata_->getInt64( 1 );
      data.type = stmt_metadata_->getDataType( 2 );
    }
    stmt_metadata_->reset();
    return data;
  }

  bool KVStore::getValue( const std::string &key, std::string &value ) const {
    bool result = true;
    stmt_getvalue_->bind( 1, key );
    if ( stmt_getvalue_->step() ) {
      value = stmt_getvalue_->getText(0);
      result = true;
    } else result = false;
    stmt_getvalue_->reset();
    return result;
  }

  bool KVStore::getValue( const std::string &key, double &value ) const {
    bool result = true;
    stmt_getvalue_->bind( 1, key );
    if ( stmt_getvalue_->step() ) {
      value = stmt_getvalue_->getDouble(0);
      result = true;
    } else result = false;
    stmt_getvalue_->reset();
    return result;
  }

  bool KVStore::getValue( const std::string &key, int64_t &value ) const {
    bool result = true;
    stmt_getvalue_->bind( 1, key );
    if ( stmt_getvalue_->step() ) {
      value = stmt_getvalue_->getInt64(0);
      result = true;
    } else result = false;
    stmt_getvalue_->reset();
    return result;
  }

  bool KVStore::getValue( const std::string &key, common::Bytes &value ) const {
    bool result = true;
    stmt_getvalue_->bind( 1, key );
    if ( stmt_getvalue_->step() ) {
      stmt_getvalue_->getBytes( 0, value );
      result = true;
    } else result = false;
    stmt_getvalue_->reset();
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const std::string &value ) {
    stmt_insert_->bind( 1, key );
    stmt_insert_->bind( 2, value );
    auto count = stmt_insert_->execute();
    stmt_insert_->reset();
    return count == 1;
  }

  bool KVStore::insertKey( const std::string &key, const double &value ) {
    bool result = true;
    stmt_insert_->bind( 1, key );
    stmt_insert_->bind( 2, value );
    stmt_insert_->execute();
    stmt_insert_->reset();
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const int64_t &value ) {
    bool result = true;
    stmt_insert_->bind( 1, key );
    stmt_insert_->bind( 2, value );
    stmt_insert_->execute();
    stmt_insert_->reset();
    return result;
  }

  bool KVStore::insertKey( const std::string &key, const common::Bytes &value ) {
    bool result = true;
    stmt_insert_->bind( 1, key );
    stmt_insert_->bind( 2, value );
    stmt_insert_->execute();
    stmt_insert_->reset();
    return result;
  }

  void KVStore::optimize() {
    sqlite::DDL ddl( *db_ );
    ddl.prepare( "PRAGMA optimize;" );
    ddl.execute();
  }

  void KVStore::prepareSQL() {
    persist::sqlite::DDL ddl( *db_ );
    ddl.prepare( "PRAGMA case_sensitive_like=ON;" );
    ddl.execute();

    stmt_exists_->prepare( "SELECT COUNT(1) FROM kvstore WHERE key = ?" );
    stmt_getvalue_->prepare( "SELECT value FROM kvstore WHERE key = ?" );
    stmt_insert_->prepare( "INSERT INTO kvstore ( key, value ) VALUES ( ?, ? )" );
    stmt_delete_->prepare( "DELETE FROM kvstore WHERE key = ?" );
    stmt_update_->prepare( "UPDATE kvstore SET value = ?, modified = ((julianday('now') - 2440587.5) * 86400.0), updates = updates + 1 WHERE key = ?" );
    stmt_key_filter_->prepare( "SELECT key FROM kvstore WHERE key LIKE ? ORDER BY key" );
    stmt_key_value_filter_->prepare( "SELECT key, value FROM kvstore WHERE key LIKE ? ORDER BY key" );
    stmt_metadata_->prepare( "SELECT modified, updates, value FROM kvstore WHERE key = ?" );
  }

  void KVStore::rollbackTransaction() {
    db_->rollback();
  }

  bool KVStore::setKey( const std::string &key, const std::string &value ) {
    stmt_update_->bind( 1, value );
    stmt_update_->bind( 2, key );
    int rows = stmt_update_->execute();
    stmt_update_->reset();
    return rows == 1;
  }

  bool KVStore::setKey( const std::string &key, const double &value ) {
    stmt_update_->bind( 1, value );
    stmt_update_->bind( 2, key );
    int rows = stmt_update_->execute();
    stmt_update_->reset();
    return rows == 1;
  }

  bool KVStore::setKey( const std::string &key, const int64_t &value ) {
    stmt_update_->bind( 1, value );
    stmt_update_->bind( 2, key );
    int rows = stmt_update_->execute();
    stmt_update_->reset();
    return rows == 1;
  }

  bool KVStore::setKey( const std::string &key, const common::Bytes &value ) {
    stmt_update_->bind( 1, value );
    stmt_update_->bind( 2, key );
    int rows = stmt_update_->execute();
    stmt_update_->reset();
    return rows == 1;
  }

  void KVStore::startTransaction() {
    sqlite::DDL ddl( *db_ );
    ddl.prepare( "BEGIN TRANSACTION" );
    ddl.execute();
  }

  void KVStore::vacuum() {
    sqlite::DDL ddl( *db_ );
    ddl.prepare( "VACUUM;" );
    ddl.execute();
  }

};