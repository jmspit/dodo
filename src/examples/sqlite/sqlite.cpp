#include <dodo.hpp>
#include <cstdio>

using namespace dodo;

int main() {
  int exit_code = 0;
  const std::string database_name = "mydb.sqlite";
  try {
    {
      // open existing or create new database
      persist::sqlite::Database db( database_name );

      // create a schema
      persist::sqlite::DDL schema( db );
      schema.prepare( "CREATE TABLE IF NOT EXISTS foo ( foo NOT NULL PRIMARY KEY)" );
      schema.execute();

      // setup an insert statement
      persist::sqlite::DML insert( db );
      insert.prepare( "INSERT INTO foo (foo) VALUES (?)" );
      insert.bind( 1, "insert1" );

      // start a transaction
      db.beginTransaction();
      insert.execute();

      // reset the query for re-execute with different value
      insert.reset(true);
      insert.bind( 1, "insert2" );
      insert.execute();

      db.commit();
      // other persist::sqlite::Database objects will now see the inserts

      // query and step the result set
      persist::sqlite::Query qry( db );
      qry.prepare( "select foo from foo order by foo" );
      while ( qry.step() ) {
        std::cout << qry.getText(0) << std::endl;
      }
    }
  }
  catch ( const std::exception &e ) {
    // as the db object is now destructed, any un-comitted transaction is rolled back and the database is closed.
    std::cerr << e.what() << std::endl;
    exit_code = 1;
  }
  // delete the database file
  remove( database_name.c_str() );
  return exit_code;
}