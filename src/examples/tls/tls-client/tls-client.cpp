#include "dodo.hpp"

using namespace dodo;

#include <buildenv.hpp>
#include <src/include/common/puts.hpp>

int main( int argc, char* argv[] ) {
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET6,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );

  network::TLSContext tlsctx( network::TLSContext::TLSVersion::tls1_1 );

  tlsctx.loadCertificate( BuildEnv::getSourceDirectory() +
                            "/examples/tls/artefacts" +
                            "/ca/root/ext/servers/localhost.cert.pem",
                          BuildEnv::getSourceDirectory() +
                            "/examples/tls/artefacts" +
                            "/ca/root/ext/servers/localhost.key.pem",
                            "6vmmoskJgT8ItUGDcCWkq7bvN9ydyW0y" );
  //tlsctx.setCipherList( common::Puts() << "TLS_AES_256_GCM_SHA384" );
  tlsctx.setCipherList( common::Puts() << "DHE-RSA-AES256-GCM-SHA384" );
  //tlsctx.setCipherList( "DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!DSS:!RC4:!SEED:!ECDSA:!ADH:!IDEA:!3DES" );
  tlsctx.setOptions( SSL_OP_CIPHER_SERVER_PREFERENCE );
  network::TLSSocket tlssocket( true, sock_params, tlsctx );
  return 0;
}
