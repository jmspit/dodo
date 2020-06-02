#include "dodo.hpp"

using namespace dodo;

#include <buildenv.hpp>
#include <src/include/common/puts.hpp>
#include <string>

int main( int argc, char* argv[] ) {
  int program_error = 0;
  try {
    dodo::initLibrary();

    network::SocketParams sock_params = network::SocketParams( network::SocketParams::afUNSPEC,
                                                               network::SocketParams::stSTREAM,
                                                               network::SocketParams::pnTCP );
    std::string canonicalname;
    std::string host = "www.gnu.org";
    if ( argc > 1 ) host = argv[1];
    uint16_t port = 443;
    if ( argc > 2 ) port = static_cast<uint16_t>( atoi( argv[2] ) );
    if ( !port ) port = 443;
    network::TLSContext::PeerVerification pv = network::TLSContext::PeerVerification::pvNone;
    if ( argc > 3 ) {
      if ( strncmp( argv[3], "pvVerifyPeer", 13 ) == 0 ) pv = network::TLSContext::PeerVerification::pvVerifyPeer;
      else if ( strncmp( argv[3], "pvVerifyFQDN", 12 ) == 0 ) pv = network::TLSContext::PeerVerification::pvVerifyFQDN;
    }
    bool enableSNI = false;
    if ( argc > 4 ) enableSNI = ( strncmp( argv[4], "true", 4 ) == 0 );



    network::Address address;
    common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
    if ( error != common::SystemError::ecOK ) throw_SystemException( common::Puts() << "cannot resolve " << host, error );

    network::TLSContext tlsctx( pv,
                                network::TLSContext::TLSVersion::tls1_2,
                                enableSNI );
    //tlsctx.setTrustPaths( BuildEnv::getSourceDirectory() + "/examples/tls/artefacts/ca/root/certs/ca.cert.pem",
    //                      BuildEnv::getSourceDirectory() + "/examples/tls/artefacts/ca/root/certs" );
    tlsctx.setOptions( SSL_OP_ALL );
    tlsctx.setCipherList( "HIGH:!ADH:!MD5:!RC4:!SRP:!PSK:!DSS" );
    network::TLSSocket tlssocket( true,
                                  sock_params,
                                  tlsctx,
                                  network::X509Certificate::SAN { network::X509Certificate::SANType::stDNS, host } );
    address.setPort(port);
    error = tlssocket.connect( address );
    if ( error == common::SystemError::ecOK ) {
      std::cout << "connected to " << tlssocket.getPeerAddress().asString(true) << std::endl;
      X509* peer_cert = tlssocket.getPeerCertificate();
      if ( peer_cert ) {
        std::cout << "Peer issuer: " << network::X509Certificate::getIssuer( peer_cert ) << std::endl;
        std::cout << "Peer subject: " << network::X509Certificate::getSubject( peer_cert ) << std::endl;
        std::cout << "Peer md5 fingerprint: " << network::X509Certificate::getFingerPrint( peer_cert, "md5" ) << std::endl;
        std::cout << "Peer address: " << tlssocket.getPeerAddress().asString() << std::endl;
        std::string peer_fqdn = "";
        if ( tlssocket.getPeerAddress().getNameInfo( peer_fqdn ) == common::SystemError::ecOK ) {
          std::cout << "Peer reverse FQDN: " << peer_fqdn << std::endl;
        }
        for ( auto n : network::X509Certificate::getSubjectAltNames( peer_cert ) ) {
          std::cout << (int)n.san_type << " " << n.san_name << std::endl;
        }
      }
      tlssocket.close();
    } else throw_SystemException( common::Puts() << "failed to connect to '" << host << "'", error );
  }
  catch ( const std::runtime_error &e ) {
    std::cerr << e.what() << std::endl;
    program_error = 1;
  }
  dodo::closeLibrary();
  return program_error;
}
