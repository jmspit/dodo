#include "dodo.hpp"

using namespace dodo;

#include <iostream>

void showPrivateKey( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type             : " << pem_tag << std::endl;
}

void showPublicKey( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type             : " << pem_tag << std::endl;
}

void showCertificate( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type             : " << pem_tag << std::endl;
  X509* cert = network::X509Certificate::loadPEM( filename );
  std::cout << "Serial               : " << network::X509Certificate::getSerial( cert ) << std::endl;
  std::cout << "Issuer               : " << network::X509Certificate::getIssuer( cert ).commonName << std::endl;
  std::cout << "Subject              : " << network::X509Certificate::getSubject( cert ).commonName << std::endl;
  std::cout << "Fingerprint (md5)    : " << network::X509Certificate::getFingerPrint( cert, "md5" ) << std::endl;
  std::cout << "Fingerprint (sha1)   : " << network::X509Certificate::getFingerPrint( cert, "sha1" ) << std::endl;
  std::cout << "Fingerprint (sha256) : " << network::X509Certificate::getFingerPrint( cert, "sha256" ) << std::endl;
  network::X509Certificate::free( cert );
}

void showCertificateSigningRequest( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type             : " << pem_tag << std::endl;
  X509_REQ* cert = network::X509CertificateSigningRequest::loadPEM( filename );
  std::cout << "Subject              : " << network::X509CertificateSigningRequest::getSubject( cert ).commonName<< std::endl;
  std::cout << "Fingerprint (md5)    : " << network::X509CertificateSigningRequest::getFingerPrint( cert, "md5" ) << std::endl;
  std::cout << "Fingerprint (sha1)   : " << network::X509CertificateSigningRequest::getFingerPrint( cert, "sha1" ) << std::endl;
  std::cout << "Fingerprint (sha256) : " << network::X509CertificateSigningRequest::getFingerPrint( cert, "sha256" ) << std::endl;
  network::X509CertificateSigningRequest::free( cert );
}

int main( int argc, char* argv[] ) {
  if ( argc != 2 ) return 1;
  try {
    std::string pem_tag = "";
    network::X509Common::X509Type type = network::X509Common::detectX509Type( argv[1], pem_tag );
    switch ( type ) {
      case network::X509Common::X509Type::Unknown:
        std::cerr << "unknown document type" << std::endl;
        return 1;
      case network::X509Common::X509Type::PrivateKey:
        showPrivateKey( argv[1], pem_tag );
        break;
      case network::X509Common::X509Type::PublicKey:
        showPublicKey( argv[1], pem_tag );
        break;
      case network::X509Common::X509Type::Certificate:
        showCertificate( argv[1], pem_tag );
        break;
      case network::X509Common::X509Type::CertificateSigningRequest:
        showCertificateSigningRequest( argv[1], pem_tag );
        break;
    }
  }
  catch ( std::exception &e  ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}