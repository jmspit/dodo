#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace std;

int main() {
  int error = 0;
  try {
    dodo::initLibrary();

    network::protocol::stomp::Connect c;
    c.setHost( "stomp.github.org" );
    c.setLogin( "spjm" );
    c.setPasscode( "secret" );
    c.setHeartbeat( 10000, 30000 );
    common::Bytes oa;
    c.generate( oa );
    std:: cout << oa.asString() << std::endl;
    std:: cout << oa.hexDump(4096) << std::endl;

  }
  catch ( const std::exception& e ) {
    cerr << e.what() << endl;
    error = 2;
  }
  dodo::closeLibrary();
  return error;
}