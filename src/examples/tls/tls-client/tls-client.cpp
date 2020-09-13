#include <dodo.hpp>

using namespace dodo;
using namespace std;

#include <buildenv.hpp>
#include <src/include/common/puts.hpp>
#include <string>

void writeIdentityLine( std::ostream &os, const std::string& caption, const std::string &value ) {
  if ( value.length() > 0 ) os << setw(21) << caption << " : " << value << endl;
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

int main( int argc, char* argv[], char** envp ) {

  int program_error = 0;
  try {
    dodo::initLibrary();

    network::SocketParams sock_params = network::SocketParams( network::SocketParams::afUNSPEC,
                                                               network::SocketParams::stSTREAM,
                                                               network::SocketParams::pnTCP );
    string canonicalname;
    string host = "www.gnu.org";
    if ( argc > 1 ) host = argv[1];
    uint16_t port = 443;
    if ( argc > 2 ) port = static_cast<uint16_t>( atoi( argv[2] ) );
    if ( !port ) port = 443;
    network::TLSContext::PeerVerification pv = network::TLSContext::PeerVerification::pvVerifyNone;
    if ( argc > 3 ) {
      if ( strncmp( argv[3], "pvVerifyPeer", 13 ) == 0 ) pv = network::TLSContext::PeerVerification::pvVerifyPeer;
      else if ( strncmp( argv[3], "pvVerifyFQDN", 12 ) == 0 ) pv = network::TLSContext::PeerVerification::pvVerifyFQDN;
    }
    bool enableSNI = true;
    if ( argc > 4 ) enableSNI = ( strncmp( argv[4], "true", 4 ) == 0 );



    network::Address address;
    common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
    if ( error != common::SystemError::ecOK ) throw_SystemException( "cannot resolve " << host, error );

    network::TLSContext tlsctx( pv,
                                network::TLSContext::TLSVersion::tls1_2,
                                enableSNI );
    //tlsctx.setTrustPaths( BuildEnv::getSourceDirectory() + "/examples/tls/artefacts/ca/root/certs/ca.cert.pem",
    //                      BuildEnv::getSourceDirectory() + "/examples/tls/artefacts/ca/root/certs" );
    tlsctx.setOptions( SSL_OP_ALL );
    //tlsctx.setCipherList( "HIGH:!ADH:!MD5:!RC4:!SRP:!PSK:!DSS" );
    //tlsctx.setCipherList( "HIGH" );
    network::X509Certificate::SAN peer_san { network::X509Certificate::SANType::stDNS, host };
    network::TLSSocket tlssocket( true,
                                  sock_params,
                                  tlsctx,
                                  peer_san );
    address.setPort(port);
    common::StopWatch sw;
    error = tlssocket.connect( address );
    sw.stop();
    if ( error == common::SystemError::ecOK ) {
      cout << "connected to " << tlssocket.getPeerAddress().asString(true) << endl;
      X509* peer_cert = tlssocket.getPeerCertificate();
      if ( peer_cert ) {
        network::X509Certificate::Identity ident = network::X509Certificate::getIssuer( peer_cert );
        cout << "Peer certificate issuer: " << endl;
        writeIdentity( cout, ident );
        ident = network::X509Certificate::getSubject( peer_cert );
        cout << "Peer certificate subject: " << endl;
        writeIdentity( cout, ident );
        cout << "Peer md5 fingerprint: " << network::X509Certificate::getFingerPrint( peer_cert, "md5" ) << endl;
        cout << "Peer address: " << tlssocket.getPeerAddress().asString() << endl;
        string peer_fqdn = "";
        if ( tlssocket.getPeerAddress().getNameInfo( peer_fqdn ) == common::SystemError::ecOK ) {
          cout << "Peer reverse FQDN: " << peer_fqdn << endl;
        }
        for ( auto n : network::X509Certificate::getSubjectAltNames( peer_cert ) ) {
          cout << (int)n.san_type << " " << n.san_name << endl;
        }
      }
      cout << "Handshake             : " << fixed << setprecision(3) << sw.getElapsedSeconds() * 1000.0 << "ms" << endl;
      cout << "SAN verification      : "
           << (network::X509Certificate::verifySAN( peer_cert, peer_san )?string("Match"):string("No match"))
           << endl;
      cout << "TLS protocol version  : " << tlssocket.getTLSProtocolVersionString() << endl;
      cout << "TLS negotiated cipher : " << tlssocket.getTLSCurrentCipherName() << endl;
      tlssocket.close();
    } else throw_SystemException( "failed to connect to '" << host << "'", error );
  }
  catch ( const runtime_error &e ) {
    cerr << e.what() << endl;
    program_error = 1;
  }
  dodo::closeLibrary();
  return program_error;
}
