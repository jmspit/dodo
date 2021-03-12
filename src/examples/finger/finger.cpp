#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace std;

void printHelp() {
  std::cout << "lists socket exposure for hostnames or ip addresses " << std::endl << std::endl;
  std::cout << "uses the getnameinfo and getaddrinfo calls (man 3 getaddrinfo, man 3 getnameinfo) " << std::endl << std::endl;
  std::cout << "usage: " << std::endl;
  std::cout << "  finger [fqdn|ip4|ip6]" << std::endl;
}

int main( int argc, char* argv[] ) {

  try {
    dodo::initLibrary();

    if ( argc < 2 )  {
      printHelp();
      return 1;
    }

    if ( strncmp( argv[1], "-", 1 ) == 0  ) {
      printHelp();
      return 0;
    }

    std::string hostname = argv[1];

    dodo::network::Address testaddr(hostname);
    if ( testaddr.isValid() ) {
      std::string reverse = "";
      common::SystemError error = testaddr.getNameInfo( reverse );
      if ( error != common::SystemError::ecOK ) {
        std::cerr << "network::Address::getNameInfo failed : " << error.asString() << std::endl;
      } else std::cout << "reverse on " << testaddr.asString() << " is " << reverse << std::endl;
    }

    dodo::network::AddrInfo info;
    common::SystemError error = network::Address::getHostAddrInfo( hostname, info );
    if ( error != common::SystemError::ecOK ) {
      std::cerr << "network::Address::getHostAddrInfo failed : " << error.asString() << std::endl;
      return 1;
    }
    std::cout << "canonical: " << info.canonicalname << std::endl;
    for ( auto const &i : info.items ) {
      std::cout << i.address.asString() << " " << i.params.asString() << std::endl;
    }
    //throw std::runtime_error("oops");

  }
  catch ( const std::exception &e ) {
    std::cerr << "exception: " << e.what() << std::endl;
    return 2;
  }
  dodo::closeLibrary();
  return 0;
}