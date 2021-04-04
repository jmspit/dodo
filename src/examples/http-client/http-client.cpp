#include <iostream>

#include <dodo.hpp>

using namespace dodo::network;
using namespace dodo::network::protocol::http;
using namespace dodo::common;

const std::string host = "httpbin.org";

struct RTStats {
  double dns_time = 0.0;
  double handshake_time = 0.0;
  double send_time = 0.0;
  double read_time = 0.0;
  size_t body_bytes = 0;
  size_t underflows = 0;
};

// argv[1] = host
// argv[2] = method
// argv[3] = uri
int main( int argc, char* argv[] ) {
  if ( argc < 4 ) return 1;
  int error = 0;
  RTStats rtstats;
  std::string host = argv[1];
  std::string method = argv[2];
  std::string uri = argv[3];
  try {
    dodo::initLibrary();

    StopWatch sw;

    SocketParams sockparams( SocketParams::AddressFamily::afUNSPEC, SocketParams::stSTREAM, SocketParams::pnTCP );
    X509Common::SAN san = {  X509Common::SANType::stDNS, host };
    Address address;
    std::string canonicalname = "";
    sw.start();
    SystemError syserr = Address::getHostAddrInfo( host, sockparams, address, canonicalname );
    rtstats.dns_time = sw.getElapsedSeconds();
    if ( syserr.ok() ) {
      std::cout << host << " = " << address.asString() << std::endl;
      TLSContext tlscontext( TLSContext::PeerVerification::pvVerifyFQDN, TLSContext::TLSVersion::tls1_1, true );
      tlscontext.setOptions( SSL_OP_ALL );
      TLSSocket sock( true, sockparams, tlscontext, san );
      sock.setReceiveBufSize( 8192*100 );
      address.setPort( 443 );
      syserr = sock.connect( address );
      rtstats.handshake_time = sw.getElapsedSeconds();
      if( syserr.ok() ) {
        HTTPRequest req;
        req.getRequestLine().setHTTPVersion( HTTPVersion(1,1) );
        req.getRequestLine().setMethod( HTTPRequest::methodFromString( method ) );
        req.getRequestLine().setRequestURI( uri );
        req.addHeader( "host", host );
        std::cout << "----------------------------------- BEGIN REQ -----------------------------------" << std::endl;
        std::cout << req.asString() << std::endl;
        std::cout << "----------------------------------- END   REQ -----------------------------------" << std::endl;
        //syserr = sock.send( req.asString().c_str(), req.asString().length() );
        syserr = req.send( &sock );
        rtstats.send_time = sw.getElapsedSeconds();
        if ( syserr.ok() ) {
          SocketReadBuffer rbuf( &sock );
          syserr = rbuf.underflow();
          if ( syserr.ok() ) {
            HTTPResponse resp;
            HTTPFragment::ParseResult parse_result = resp.parse( rbuf );
            rtstats.read_time = sw.getElapsedSeconds();
            if ( parse_result.ok() ) {
              rtstats.underflows = rbuf.getUnderflowCount();
              std::cout << "----------------------------------- BEGIN RSP -----------------------------------" << std::endl;
              std::cout << resp.asString() << std::endl;
              rtstats.body_bytes = resp.asString().length();
              std::cout << "----------------------------------- END   RSP -----------------------------------" << std::endl;
            } else {
              std::cerr << "invalid HTTP response: " << parse_result.asString() << std::endl;
              error = 1;
            }
          } else {
            std::cerr << "socket read error: " << syserr.asString() << std::endl;
            error = 1;
          }
          rtstats.read_time = sw.getElapsedSeconds();

        } else {
          std::cerr << "send failed: " << syserr.asString() << std::endl;
          error = 1;
        }

      } else {
        std::cerr << "connect failed: " << syserr.asString() << std::endl;
        error = 1;
      }
    } else {
      std::cerr << "resolving of " << host << " failed : " << syserr.asString() << std::endl;
    }
  }
  catch ( const std::exception &e ) {
    std::cerr << e.what() << std::endl;
    error =  1;
  }
  std::cout << "DNS       " << std::fixed << std::setprecision(3) << rtstats.dns_time*1000 << "ms" << std::endl;
  std::cout << "HANDSHAKE " << std::fixed << std::setprecision(3) << rtstats.handshake_time*1000 << "ms" << std::endl;
  std::cout << "SEND      " << std::fixed << std::setprecision(3) << rtstats.send_time*1000 << "ms" << std::endl;
  std::cout << "RECEIVE   " << std::fixed << std::setprecision(3) << rtstats.read_time*1000 << "ms" << std::endl;
  std::cout << "RSP body size " << std::fixed << std::setprecision(0) << rtstats.body_bytes << "B" << std::endl;
  std::cout << "receive underflows " << std::fixed << std::setprecision(0) << rtstats.underflows << std::endl;
  if ( rtstats.underflows ) std::cout << "receive bytes / roundtrip " << std::fixed << std::setprecision(0) << rtstats.body_bytes/rtstats.underflows << std::endl;
  dodo::closeLibrary();
  return error;
}