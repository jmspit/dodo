#include <iostream>

#include <dodo.hpp>

using namespace dodo;
using namespace std;


#define KEY_SPACE 160000
#define KEY_MAX_LENGTH 64
#define DATA_MAX_LENGTH 128
#define MAX_SETKEYS 100

set<string> keys;

string random_string( size_t length ) {
  auto randchar = []() -> char
  {
      const char charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[ rand() % max_index ];
  };
  string str(length,0);
  generate_n( str.begin(), length, randchar );
  return str;
}

void setupTestData() {
  for ( size_t i = 0; i < KEY_SPACE; i++ ) {
    keys.insert( random_string( rand() % KEY_MAX_LENGTH ) );
  }
}

void insertKeys( persist::KVStore &store ) {
  store.startTransaction();
  for ( const auto &k : keys ) {
    store.insertKey( k, random_string( rand() % DATA_MAX_LENGTH ) );
  }
  store.commitTransaction();
}

void fetchKeys( persist::KVStore &store ) {
  for ( const auto &k : keys ) {
    string value = "";
    if ( ! store.getValue( k, value ) ) throw runtime_error( "key not found - cannot be!" );
  }
}

void updateKeys( persist::KVStore &store ) {
  size_t count = 0;
  for ( const auto &k : keys ) {
    store.setKey( k, random_string( rand() % DATA_MAX_LENGTH ) );
    if ( ++count > MAX_SETKEYS ) break;
  }
}

int main() {

  try {
    persist::KVStore store( "kvstore.db" );
    common::StopWatch sw;
    double time_setup = 0.0;
    double time_insert = 0.0;
    double time_checkpoint = 0.0;
    double time_fetch = 0.0;
    double time_update = 0.0;
    sw.start();
    setupTestData();
    time_setup = sw.restart();
    insertKeys( store );
    time_insert = sw.restart();
    store.checkpoint();
    time_checkpoint = sw.restart();
    fetchKeys( store );
    time_fetch = sw.restart();
    updateKeys( store );
    time_update = sw.stop();
    cout << "setup " << time_setup << "s" << endl;
    cout << "insertKey (bulk) " << time_insert << "s" << endl;
    cout << "checkpoint " << time_checkpoint << "s" << endl;
    cout << "getValue " << time_fetch << "s" << endl;
    cout << "setKey (single) " << time_update << "s" << endl;
    cout << static_cast<double>(keys.size())/time_insert << " insertKey (bulk) per second" << endl;
    cout << static_cast<double>(keys.size())/time_fetch << " getValue per second" << endl;
    cout << static_cast<double>(MAX_SETKEYS)/time_update << " setKey (single) per second" << endl;
  }
  catch ( const runtime_error  &e ) {
    cerr << e.what() << endl;
  }
  return 0;
}