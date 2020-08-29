#include "dodo.hpp"


using namespace dodo;
using namespace dodo::common;
using namespace dodo::store::kvstore;
using namespace std;

int main() {
  KVStore kvstore;
  SystemError error = kvstore.init( "test.kvstore", 8192, 100, KVStore::ShareMode::Shared );
  //SystemError error = kvstore.open( "test.kvstore", KVStore::ShareMode::Private );
  bool ok = kvstore.analyze( std::cout );
  return !ok && error;
}