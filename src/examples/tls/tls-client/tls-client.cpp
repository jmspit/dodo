#include "dodo.hpp"

using namespace dodo;

int main( int argc, char* argv[] ) {
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET6,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );

  network::TLSContext tlsctx;

  tlsctx.loadCertificates( "../examples/network/sslkeys/server.pem", "../examples/network/sslkeys/server.key", "" );
  tlsctx.setCipherList( "DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!DSS:!RC4:!SEED:!ECDSA:!ADH:!IDEA:!3DES" );
  tlsctx.setOptions( SSL_OP_CIPHER_SERVER_PREFERENCE );
  network::TLSSocket tlssocket( true, sock_params, tlsctx );

  return 0;
}
