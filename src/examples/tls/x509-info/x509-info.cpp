#include <dodo.hpp>

using namespace dodo;

#include <iostream>

void writeIdentityLine( std::ostream &os, const std::string& caption, const std::string &value ) {
  if ( value.length() > 0 ) os << std::setw(21) << caption << " : " << value << std::endl;
}

void writeIdentity( std::ostream &os, const network::X509Certificate::Identity &ident ) {
  writeIdentityLine( os, "organization", ident.organization );
  writeIdentityLine( os, "organization unit", ident.organizationUnit );
  writeIdentityLine( os, "street", ident.street );
  writeIdentityLine( os, "postal code", ident.postalCode );
  writeIdentityLine( os, "locality", ident.locality );
  writeIdentityLine( os, "state", ident.state );
  writeIdentityLine( os, "country", ident.countryCode );
  writeIdentityLine( os, "common name", ident.commonName );
  writeIdentityLine( os, "email", ident.email );
  writeIdentityLine( os, "business category", ident.businessCategory );
  writeIdentityLine( os, "jurisdiction state", ident.jurisdictionST );
  writeIdentityLine( os, "jurisdiction country", ident.jurisdictionC );
  writeIdentityLine( os, "serial number", ident.serialNumber );
  for ( auto i : ident.other ) {
    writeIdentityLine( os, i.first, i.second );
  }
}

void showPrivateKey( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type             : " << pem_tag << std::endl;
}

void showPublicKey( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type             : " << pem_tag << std::endl;
}

void showCertificate( const std::string &filename, const std::string &pem_tag ) {
  std::cout << "PEM type              : " << pem_tag << std::endl;
  X509* cert = network::X509Certificate::loadPEM( filename );
  std::cout << "Serial                : " << network::X509Certificate::getSerial( cert ) << std::endl;
  std::cout << "Issuer                : " << std::endl;
  writeIdentity( std::cout, network::X509Certificate::getIssuer( cert ) );
  std::cout << "Subject               : " << std::endl;
  writeIdentity( std::cout, network::X509Certificate::getSubject( cert ) );
  std::cout << "Fingerprint (md5)     : " << network::X509Certificate::getFingerPrint( cert, "md5" ) << std::endl;
  std::cout << "Fingerprint (sha1)    : " << network::X509Certificate::getFingerPrint( cert, "sha1" ) << std::endl;
  std::cout << "Fingerprint (sha256)  : " << network::X509Certificate::getFingerPrint( cert, "sha256" ) << std::endl;
  std::cout << "SAN entries           : " << std::endl;
  std::list< network::X509Common::SAN > altnames = network::X509Certificate::getSubjectAltNames( cert );
  for ( auto a : altnames ) {
    std::cout << "                      : " << a.san_name << " SAN type " << network::X509Common::SANTypeAsString( a.san_type ) << std::endl;
  }
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