#include <iostream>
#include <dodo.hpp>

using namespace dodo;

int main() {
  try {
    common::Config* config = common::Config::initialize( BuildEnv::getBinaryDirectory() + "/test-network-tlscontext.yaml" );
    network::TLSContext tlscontext( *config, {"tlscontext"} );
  } catch ( const std::exception &e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}