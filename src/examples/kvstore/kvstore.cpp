#include "dodo.hpp"


using namespace dodo;
using namespace dodo::common;
using namespace dodo::store;
using namespace std;

int main() {
  KVStore kvstore;
  SystemError error = kvstore.init( "test.kvstore", 1024 * 128 );
  kvstore.analyze( std::cout );
  if ( error != SystemError::ecOK ) return 1;
}