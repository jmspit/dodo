#include <iostream>
#include <dodo.hpp>

#include <iostream>
#include <common/unittest.hpp>

using namespace dodo;

class AddressTest : public common::UnitTest {
  public:
    AddressTest( const string &name, const string &description, ostream *out ) :
      UnitTest( name, description, out ) {};
  protected:
    virtual void doRun();

    bool test1();
    bool test2();
    bool test3();
    bool test4();
    bool test5();
    bool test6();
    bool test7();
};

void AddressTest::doRun() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
}

bool AddressTest::test1() {
  network::Address address;
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afUNSPEC,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  std::string canonicalname;
  std::string host = "www.gnu.org";
  common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
  std::cout << host << " " << "cname=" << canonicalname << " ip=" << address.asString() << std::endl;
  return writeSubTestResult( "test getHostAddrInfo", "test resolving of www.gnu.org", error == common::SystemError::ecOK );
}

bool AddressTest::test2() {
  network::Address address;
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  std::string canonicalname;
  std::string host = "localhost";
  common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
  std::cout << host << " " << "cname=" << canonicalname << " ip=" << address.asString() << std::endl;
  return writeSubTestResult( "test getHostAddrInfo",
                             "test resolving of localhost to ipv4 address",
                             address.isValid() &&
                             error == common::SystemError::ecOK &&
                             address.asString() == "127.0.0.1" );
}

bool AddressTest::test3() {
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
  return writeSubTestResult( "test getHostAddrInfo",
                             "test resolving of localhost to ipv6 address",
                             error == common::SystemError::ecOK && address.asString() == "::1" );
}

bool AddressTest::test4() {
  network::Address address;
  string sadres = "127.0.0.1";
  address = sadres;
  return writeSubTestResult( "test assign string to Address",
                             common::Puts() << "test assign " << address.asString() << " to Address",
                             address.isValid() &&
                             address.getAddressFamily() == network::SocketParams::afINET &&
                             address.asString() == sadres );
}

bool AddressTest::test5() {
  network::Address address;
  string sadres = "::1";
  address = sadres;
  return writeSubTestResult( "test assign string to Address",
                             common::Puts() << "test assign " << sadres << " to Address",
                             address.isValid() &&
                             address.getAddressFamily() == network::SocketParams::afINET6 &&
                             address.asString() == sadres );
}

bool AddressTest::test6() {
  network::Address one = std::string("127.0.0.1");
  network::Address two = std::string("127.0.0.1");
  return writeSubTestResult( "test Address equality ipv4",
                             common::Puts() << "test compare Address",
                             one.isValid() &&
                             two.isValid() &&
                             one == two );
}

bool AddressTest::test7() {
  network::Address one = std::string("::1");
  network::Address two = std::string("0:0:0:0:0:0:0:1");
  return writeSubTestResult( "test Address equality ipv6",
                             common::Puts() << "test compare Address",
                             one.isValid() &&
                             two.isValid() &&
                             one == two );
}


int main() {
  AddressTest test( "network::Address tests", "Testing Adress class", &cout );
  return test.run() == false;
}