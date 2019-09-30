#include <iostream>
#include <dodo.hpp>

using namespace dodo;

#include <iostream>

bool test1() {
  network::Address address;
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afUNSPEC,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  std::string canonicalname;
  std::string host = "www.gnu.org";
  common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
  std::cout << host << " " << "cname=" << canonicalname << " ip=" << address.asString() << std::endl;
  return error == common::SystemError::ecOK;
}

bool test2() {
  network::Address address;
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  std::string canonicalname;
  std::string host = "localhost";
  common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
  std::cout << host << " " << "cname=" << canonicalname << " ip=" << address.asString() << std::endl;
  return error == common::SystemError::ecOK && address.asString() == "127.0.0.1";
}

bool test3() {
  network::Address address;
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET6,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  std::string canonicalname;
  std::string host = "localhost";
  common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
  if ( error == common::SystemError::ecEAI_NONAME || error == common::SystemError::ecEAI_NODATA ) {
    host = "localhost6";
    error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
  }
  std::cout << host << " " << "cname=" << canonicalname << " ip=" << address.asString() << std::endl;
  return error == common::SystemError::ecOK && address.asString() == "::1";
}



int main() {
  std::cout << BuildEnv::getDescription();
  bool ok = true;

  ok = ok && test1();
  if ( !ok ) return 1;

  ok = ok && test2();
  if ( !ok ) return 1;

  ok = ok && test3();
  if ( !ok ) return 1;

  return 0;
}