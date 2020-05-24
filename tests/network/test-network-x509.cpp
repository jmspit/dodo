#include <iostream>
#include <dodo.hpp>

using namespace dodo;

bool test1() {
  try {
    X509* cert = network::X509Certificate::loadPEM( BuildEnv::getSourceDirectory() +
                                                    "/examples/tls/artefacts/ca/root/ext/servers/localhost.cert.pem" );
    std::cout << "Issuer:  " << network::X509Certificate::getIssuer( cert ) << std::endl;
    std::cout << "Subject: " << network::X509Certificate::getSubject( cert ) << std::endl;
    for ( auto san : network::X509Certificate::getSubjectAltNames( cert ) ) {
      std::cout << san.san_type << " " << san.san_name << std::endl;
    }
    network::X509Certificate::free( cert );
  }
  catch ( const std::runtime_error &e ) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}

bool test2() {
  return true;
}


int main() {
  std::cout << BuildEnv::getDescription();
  bool ok = true;

  ok = ok && test1();
  if ( !ok ) return 1;

  ok = ok && test2();
  if ( !ok ) return 1;

  return 0;
}