#include <dodo.hpp>


using namespace dodo;
using namespace dodo::common;
using namespace dodo::store::kvstore;
using namespace std;

int main() {
  KVStore kvstore;
  SystemError error = kvstore.init( "test.kvstore", 4096, 30 );
  kvstore.extend( 10 );
  //SystemError error = kvstore.open( "test.kvstore", KVStore::ShareMode::Private );
  bool ok = kvstore.analyze( std::cout );
  return !ok && error;
}