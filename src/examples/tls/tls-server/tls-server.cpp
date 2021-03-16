#include <dodo.hpp>

using namespace dodo;
using namespace std;

#include <iostream>

int main( int argc, char* argv[] ) {
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET6,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );

  network::Address listen_address( "::1", 6666 );
  try {
    if ( !listen_address.isValid() ) throw_Exception( "invalid listen address : " << listen_address.asString() );

    network::TLSContext tlsctx;
    tlsctx.loadPEMIdentity( "../examples/network/sslkeys/server.pem",
                            "../examples/network/sslkeys/server.key",
                            "" );
    tlsctx.setCipherList( "DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!DSS:!RC4:!SEED:!ECDSA:!ADH:!IDEA:!3DES" );
    tlsctx.setOptions( SSL_OP_CIPHER_SERVER_PREFERENCE );
    network::TLSSocket listen_socket( true,
                                      sock_params,
                                      tlsctx,
                                      network::X509Certificate::SAN { network::X509Certificate::SANType::stDNS, "" } );
  }
  catch ( const std::runtime_error &e ) {
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
