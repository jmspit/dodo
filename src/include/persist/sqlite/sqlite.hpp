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
 * @file
 * SQLite3 wrapper c++ header file.
 */

#ifndef dodo_persist_sqlite_hpp
#define dodo_persist_sqlite_hpp

#include <string>
#include <sqlite3.h>
#include <common/bytes.hpp>

namespace dodo::persist {

  /**
   * C++ API to SQLite.
   */
  namespace sqlite {

    typedef int(*WaitHandler)(void*,int);

    /**
     * A STL friendly wrapper around the great sqlite3.
     * A few extension functions are added for use in SQL
     * - power(double x,double y) gives base x to the power y (x^y)
     * - floor(double x) gives the nearest integer downwards
     * - ceil(double x) gives the nearest integer upwards
     */
    class Database {
      public:

        /**
         * Constructor with explicit wait handler.
         * @param filename the database filename
         * @param handler the wait handler
         */
        Database( const std::string &filename, WaitHandler handler = 0 );

        /**
         * Destructor.
         */
        ~Database();

        /**
         * Begin a transaction.
         * @see https://sqlite.org/lang_transaction.html
         */
        void beginTransaction();

        /**
         * Begin an immediate transaction.
         * @see https://sqlite.org/lang_transaction.html
         */
        void beginImmediateTransaction();

        /**
         * Begin an exclusive transaction.
         * @see https://sqlite.org/lang_transaction.html
         */
        void beginExclusiveTransaction();

        /**
         * Issue a full checpoint.
         */
        void checkPointFull();

        /**
         * Issue a passive checpoint.
         */
        void checkPointPassive();

        /**
         * Issue a (WAL) truncate checpoint.
         */
        void checkPointTruncate();

        /** Commit a transaction. */
        void commit();

        /**
         * Disable foreign key constraints.
         */
        void disableForeignKeys();

        /**
         * Enable foreign key constraints.
         */
        void enableForeignKeys();

        /**
         * Enable triggers.
         */
        void enableTriggers();

        /**
         * Return the database filename.
         */
        std::string getFileName() const { return sqlite3_db_filename( database_, "main" ); };

        /**
         * Return database handle.
         * @return the sqlite3 database handle.
         */
        sqlite3* getDB() const { return database_; };

        /**
         * get the current user_version pragma
         */
        int getUserVersion() const;

        /**
         * Get the rowid of the last inserted row.
         */
        int64_t lastInsertRowid() const;

        /**
         * Get memory in use by the SQLite library.
         */
        static int64_t memUsed() {
          return sqlite3_memory_used();
        }

        /**
         * Get memory highwater by the SQLite library.
         */
        static int64_t memHighWater() {
          return sqlite3_memory_highwater(0);
        }

        /**
         * Release a savepoint.
         * @param sp the savepoint name
         */
        void releaseSavepoint( const std::string &sp );

        /**
         * Have SQLite attempt to release a much memory as possible.
         */
        void releaseMemory();

        /** Rollback a transaction. */
        void rollback();

        /**
         * Create a named savepoint.
         * @param sp the savepoint name
         * @see release, rollback
         */
        void createSavepoint( const std::string &sp );

        /**
         * Rollback to a savepoint.
         * @param sp the savepoint name
         */
        void rollback( const std::string &sp );

        /**
         * set the user_version pragma
         */
        void setUserVersion( int version );

        /**
         * Get the SQLite soft heap limti.
         */
        static int64_t softHeapLimit( int64_t limit ) {
          return sqlite3_soft_heap_limit64( limit );
        }

      protected:
        /** The SQLite database handle. */
        sqlite3 *database_;
    };

    /**
     * Generic SQL Statement.
     */
    class Statement {
      public:

        /**
         * Constructor.
         */
        Statement( const Database& db );

        /**
         * Destructor.
         * Virtual so that this destructor will be called when
         * descendants are deleted.
         */
        virtual ~Statement();

        /**
         * Prepare a SQL statement.
         * @param sql the SQL statement.
         */
        void prepare( const std::string &sql );

        /**
         * Reset a SQL statement for re-execute or even re-prepare.
         * @param clear If true (default), unbind the bind variables. If the statement is going to be
         * re-executed, new bind calls must be made. If the satemetn is going to be re-executed with
         * the same bind values, cler can be set to false and no re-binding is required.
         */
        void reset( bool clear = true );

        /**
         * A statement handle can be explicitly closed without deleting
         * the Statement object itself. This frees the resources in SQLite,
         * . it's allowed to call prepare() again.
         */
        void close();

      protected:

        /** statement handle. */
        sqlite3_stmt *stmt_;

        /** database handle on which the stmt_ is created. */
        sqlite3      *database_;

    };

    /**
     * Data Definition Language, SQL that takes no parameters,
     * returns no data such as CREATE TABLE.
     */
    class DDL : public Statement {
      public:

        /** Constructor. */
        DDL( const Database& db ) : Statement( db ) {};

        /** Destructor. */
        virtual ~DDL() {};

        /**
         * execute, throws Oops on error.
         * @see execute_r
         */
        void execute();

        /**
         * execute and return result code.
         * @see execute
         */
        int execute_r();
    };

    /**
     * Data Modification Language statements can take bind values.
     * Note that calling reset leaves set bind values intact.
     */
    class DML : public Statement {
      public:

        /** Constructor. */
        DML( const Database& db );

        /** Destructor. */
        virtual ~DML() {};

        /**
         * Execute and return the number of rows affected.
         * @return The number of rows affected by the DML statement.
         */
        int execute();

        /**
         * Bind a double value to the bind at position.
         * @param position the bind position in the SQL (start with 1)
         * @param value the value to bind.
         */
        void bind( int position, double value );

        /**
         * Bind an int value to the bind at position.
         * @param position the bind position in the SQL (start with 1)
         * @param value the value to bind.
         */
        void bind( int position, int value );

        /**
         * Bind a int64_t value to the bind at position.
         * @param position the bind position in the SQL (start with 1)
         * @param value the value to bind.
         */
        void bind( int position, int64_t value );

        /**
         * Bind a string value to the bind at position.
         * @param position the bind position in the SQL (start with 1)
         * @param value the value to bind.
         */
        void bind( int position, const std::string &value );

        /**
         * Bind an Bytes value to the bind at position.
         * @param position the bind position in the SQL (start with 1)
         * @param value the value to bind.
         */
        void bind( int position, const common::Bytes &value );
    };

    /**
     * Queries can take bind values and return select lists.
     */
    class Query : public DML {
      public:

        /**
         * The data type of a select-list value.
         */
        enum DataType {
          dtInteger = SQLITE_INTEGER, /**< (1) Integer type (int64_t) */
          dtFloat   = SQLITE_FLOAT,   /**< (2) Floating point type (double) */
          dtText    = SQLITE3_TEXT,   /**< (3) Text type (string) */
          dtBlob    = SQLITE_BLOB,    /**< (4) Binary data type (Bytes) */
          dtNull    = SQLITE_NULL,    /**< (5) NULL type (unset and undefined) */
          dtUnknown = 91,             /**< Used to indicate a DataType that could not be determined */
        };

        /** Constructor. */
        Query( const Database& db ) : DML( db ) {};

        /** Destructor. */
        virtual ~Query() {};

        /**
         * Step the result list, end of list returns false.
         * @return true as long as there are more rows.
         */
        bool step();

        /**
         * Test if the result is NULL
         * @param col the select list column (start with 0).
         * @return true if the col holds a NULL value
         */
        bool isNull( int col ) const;

        /**
         * Get the dataype of a select list column.
         */
        DataType getDataType( int col ) const;

        /**
         * Get int value from select list.
         * @param col the select list column (start with 0).
         * @return the select list value.
         */
        int getInt( int col ) const;

        /**
         * Get int64_t value from select list.
         * @param col the select list column (start with 0).
         * @return the select list value.
         */
        int64_t getInt64( int col ) const;

        /**
         * Get double value from select list.
         * @param col the select list column (start with 0).
         * @return the select list value.
         */
        double getDouble( int col ) const;

        /**
         * Get string value from select list.
         * @param col the select list column (start with 0).
         * @return the select list value.
         */
        std::string getText( int col ) const;

        /**
         * Get Bytes value from select list.
         * @param col the select list column (start with 0).
         * @param bytes The Bytes value to get.
         */
        void getBytes( int col, common::Bytes &bytes ) const;

        /**
         * get the number of columns in the query result set.
         * @return the number of columns
         */
        int getColumnCount() const;
    };


  }; // namespace persist

}; // namespace leanux

#endif
