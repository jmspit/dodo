#include "dodo.hpp"

using namespace dodo;

#include <buildenv.hpp>
#include <src/include/common/puts.hpp>

int main( int argc, char* argv[] ) {
  try {
    network::SocketParams sock_params = network::SocketParams( network::SocketParams::afUNSPEC,
                                                               network::SocketParams::stSTREAM,
                                                               network::SocketParams::pnTCP );
    std::string canonicalname;
    std::string host = "localhost";
    network::Address address;
    common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );

    network::TLSContext tlsctx( network::TLSContext::PeerVerification::pvNone,
                                network::TLSContext::TLSVersion::tls1_3 );
    tlsctx.setTrustPaths( BuildEnv::getSourceDirectory() + "/examples/tls/artefacts/ca/root/certs/ca.cert.pem", "" );

    //tlsctx.loadPEMIdentity( BuildEnv::getSourceDirectory() +
                              //"/examples/tls/artefacts" +
                              //"/ca/root/ext/servers/localhost.cert.pem",
                            //BuildEnv::getSourceDirectory() +
                              //"/examples/tls/artefacts" +
                              //"/ca/root/ext/servers/localhost.key.pem",
                              //"6vmmoskJgT8ItUGDcCWkq7bvN9ydyW0y" );
    //tlsctx.loadPKCS12( BuildEnv::getSourceDirectory() +
                       //"/examples/tls/artefacts" +
                       //"/ca/root/ext/servers/localhost.pkcs12",
                       //"6vmmoskJgT8ItUGDcCWkq7bvN9ydyW0y" );
    //tlsctx.setCipherList( common::Puts() << "TLS_AES_256_GCM_SHA384" );
    tlsctx.setOptions( SSL_OP_CIPHER_SERVER_PREFERENCE );
    network::TLSSocket tlssocket( true, sock_params, tlsctx );

    if ( error == common::SystemError::ecOK ) {
      address.setPort( 443 );
      error = tlssocket.connect( address );
      if ( error == common::SystemError::ecOK ) {
        std::cout << "connected to " << address.asString(true) << std::endl;
        X509* peer_cert = tlssocket.getPeerCertificate();
        if ( peer_cert ) {
          std::cout << "Peer issuer: " << network::X509Certificate::getIssuer( peer_cert ) << std::endl;
          std::cout << "Peer subject: " << network::X509Certificate::getSubject( peer_cert ) << std::endl;
          std::cout << "Peer md5 fingerprint: " << network::X509Certificate::getFingerPrint( peer_cert, "md5" ) << std::endl;
          for ( auto n : network::X509Certificate::getSubjectAltNames( peer_cert ) ) {
            std::cout << n.san_name << std::endl;
          }
        }
        return 0;
      } else throw_SystemException( common::Puts() << "failed to connect to '" << host << "'", error );
    } else throw_SystemException( common::Puts() << "failed to resolve '" << host << "'", error );
  }
  catch ( const std::runtime_error &e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
