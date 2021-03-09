#include <iostream>
#include <dodo.hpp>

#include <fstream>
#include <unistd.h>

using namespace dodo;
using namespace std;

#include <dodo.hpp>

int main() {

  try {
    dodo::persist::KVStore store( "kvstore.db" );

    cout << "insert ok : " << store.insertKey( "supply-chain.distribution-centers.tilburg.id", "007" ) << endl;
    cout << "insert ok : " << store.insertKey( "supply-chain.distribution-centers.tilburg.optimization.timeout-seconds", 360L ) << endl;

    cout << "key 'foo' exists? " << store.exists( "foo" ) << endl;
    std::string svalue;
    if ( store.getValue( "foo", svalue ) )  {
      cout << "value: " << svalue << endl;
    }
    cout << "insert ok : " << store.insertKey( "personal.security.code", 252979258343 ) << endl;
    cout << "insert ok : " << store.insertKey( "Newton", 0.98 ) << endl;
    cout << "insert ok : " << store.insertKey( "bert", "ernie" ) << endl;
    cout << "insert ok : " << store.insertKey( "Donald", "duck" ) << endl;

    common::OctetArray oa("{\"key\":\"value\"}");
    cout << "insert ok : " << store.insertKey( "OctetArray", oa ) << endl;

    unsigned char data[4] = { 10, 11, 12, 13 };
    cout << "insert ok : " << store.insertKey( "binary", &data, 4 ) << endl;
    data[0] = 9;
    cout << "setKey ok : " << store.setKey( "binary", &data, 4 ) << endl;

    if ( store.getValue( "binary", oa ) )  {
      for ( size_t i = 0; i < oa.getSize(); i++ ) {
        cout << "binary value: " << static_cast<int>(oa.getOctet(i)) << endl;
      }
    }

    double dvalue = 0.0;
    if ( store.getValue( "Newton", dvalue ) )  {
      cout << "Newton value: " << dvalue << endl;
    }

    int64_t ivalue = 0.0;
    if ( store.getValue( "personal.security.code", ivalue ) )  {
      cout << "ivalue: " << ivalue << endl;
    }

    int64_t fvalue = 0.0;
    if ( store.getValue( "Donald", fvalue ) )  {
      cout << "fvalue: " << fvalue << endl;
    }

    cout << "delete ok : " << store.deleteKey( "bert" ) << endl;
    cout << "delete ok : " << store.deleteKey( "ernie" ) << endl;

    cout << "update ok : " << store.setKey( "Donald", "Duck" ) << endl;
    cout << "update ok : " << store.setKey( "Newton", 0.99 ) << endl;
    cout << "update ok : " << store.setKey( "Newton", 0.97 ) << endl;
    cout << "update ok : " << store.setKey( "personal.security.code", 252979258342 ) << endl;

    std::list<std::string> keys;
    store.filterKeys( keys, "%na%" );
    for ( auto k : keys ) {
      cout << k << endl;
    }

  }
  catch ( const dodo::common::Exception &e ) {
    cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}