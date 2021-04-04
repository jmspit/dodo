#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace std;

bool test1() {
  std::cout << "parse HTTPVersion ... " << std::endl;
  network::protocol::http::HTTPVersion version;
  network::StringReadBuffer sbuf( "HTTP/1.2\r\n" );
  network::protocol::http::HTTPFragment::ParseResult result = version.parse( sbuf );
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( version.getMajor()!= 1 || version.getMinor() != 2  ) {
    std::cout << "error HTTP/1.2 does not match major=" << version.getMajor()
              << " minor=" << version.getMinor() << std::endl;
    return false;
  }
  std::cout << "OK" << std::endl;
  return true;
}

bool test2() {
  std::cout << "parse HTTPRequestLine ... " << std::endl;
  network::protocol::http::HTTPRequest::HTTPRequestLine line;
  network::StringReadBuffer sbuf( "GET / HTTP/1.1\r\n\n" );
  network::protocol::http::HTTPFragment::ParseResult result = line.parse( sbuf );
  if ( line.getHTTPVersion().getMajor()!= 1 || line.getHTTPVersion().getMinor() != 1  ) {
    std::cout << "error HTTP VERSION does not match major=" << line.getHTTPVersion().getMajor()
              << " minor=" << line.getHTTPVersion().getMinor() << std::endl;
    return false;
  }
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( line.getMethod() != 0 ) {
    std::cout << "method parse failure" << std::endl;
    return false;
  }
  if ( line.getRequestURI() != "/" ) {
    std::cout << "URI parse failure" << std::endl;
    return false;
  }
  std::cout << "OK" << std::endl;
  return true;
}

bool test3() {
  std::cout << "parse HTTPRequest ... " << std::endl;
  network::protocol::http::HTTPRequest request;
  network::StringReadBuffer sbuf( "GET /index.html HTTP/1.1\r\nHost: www.knmi.nl\r\nUser-Agent: curl/7.74.0\r\nAccept: */*\r\n\r\n" );

  network::protocol::http::HTTPFragment::ParseResult result = request.parse( sbuf );
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }

  if ( request.getRequestLine().getHTTPVersion().getMajor()!= 1 || request.getRequestLine().getHTTPVersion().getMinor() != 1  ) {
    std::cout << "error HTTP VERSION does not match major=" << request.getRequestLine().getHTTPVersion().getMajor()
              << " minor=" << request.getRequestLine().getHTTPVersion().getMinor() << std::endl;
    return false;
  }
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( request.getRequestLine().getMethod() != 0 ) {
    std::cout << "method parse failure" << std::endl;
    return false;
  }
  if ( request.getRequestLine().getRequestURI() != "/index.html" ) {
    std::cout << "URI parse failure" << std::endl;
    return false;
  }
  std::cout << "OK" << std::endl;
  return true;
}

bool test4() {
  std::cout << "parse HTTPRequest with body (content-length) ... " << std::endl;
  network::protocol::http::HTTPRequest request;
  network::FileReadBuffer sbuf( BuildEnv::getSourceDirectory() + "/../tests/network/http/get-body.http" );
  network::protocol::http::HTTPFragment::ParseResult result = request.parse( sbuf );
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }

  if ( request.getRequestLine().getHTTPVersion().getMajor()!= 1 || request.getRequestLine().getHTTPVersion().getMinor() != 1  ) {
    std::cout << "error HTTP VERSION does not match major=" << request.getRequestLine().getHTTPVersion().getMajor()
              << " minor=" << request.getRequestLine().getHTTPVersion().getMinor() << std::endl;
    return false;
  }
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( request.getRequestLine().getMethod() != 0 ) {
    std::cout << "method parse failure" << std::endl;
    return false;
  }
  if ( request.getRequestLine().getRequestURI() != "/index.html" ) {
    std::cout << "URI parse failure" << std::endl;
    return false;
  }
  if ( request.getBody().asString() != "0123456789" ) {
    std::cout << "body parse failure" << std::endl;
    return false;
  }

  std::cout << "OK" << std::endl;

  return true;
}

bool test5() {
  std::cout << "parse HTTPRequest with body (chunked) ... " << std::endl;
  network::protocol::http::HTTPRequest request;
  network::FileReadBuffer sbuf( BuildEnv::getSourceDirectory() + "/../tests/network/http/get-body-chunked.http" );
  network::protocol::http::HTTPFragment::ParseResult result = request.parse( sbuf );
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }

  if ( request.getRequestLine().getHTTPVersion().getMajor()!= 1 || request.getRequestLine().getHTTPVersion().getMinor() != 1  ) {
    std::cout << "error HTTP VERSION does not match major=" << request.getRequestLine().getHTTPVersion().getMajor()
              << " minor=" << request.getRequestLine().getHTTPVersion().getMinor() << std::endl;
    return false;
  }
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( request.getRequestLine().getMethod() != 0 ) {
    std::cout << "method parse failure" << std::endl;
    return false;
  }
  if ( request.getRequestLine().getRequestURI() != "/index.html" ) {
    std::cout << "URI parse failure" << std::endl;
    return false;
  }
  if ( request.getBody().asString() != "aaaabbbbbbbbcccccccccc" ) {
    std::cout << "body parse failure (" << request.getBody().asString() << ")" << std::endl;
    return false;
  }

  std::cout << "OK" << std::endl;

  return true;
}

bool test6() {
  std::cout << "parse HTTPResponseLine ... " << std::endl;
  network::protocol::http::HTTPResponse::HTTPResponseLine line;
  network::StringReadBuffer sbuf( "HTTP/1.1 200 OK\r\n\n" );
  network::protocol::http::HTTPFragment::ParseResult result = line.parse( sbuf );
  if ( line.getHTTPVersion().getMajor()!= 1 || line.getHTTPVersion().getMinor() != 1  ) {
    std::cout << "error HTTP VERSION does not match major=" << line.getHTTPVersion().getMajor()
              << " minor=" << line.getHTTPVersion().getMinor() << std::endl;
    return false;
  }
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( line.getHTTPCode() != 200 ) {
    std::cout << "HTTPCode parse failure" << std::endl;
    return false;
  }
  std::cout << "OK" << std::endl;
  return true;
}

bool test7() {
  std::cout << "parse HTTPResponse with body (content-length) ... " << std::endl;
  network::protocol::http::HTTPResponse response;
  network::FileReadBuffer sbuf( BuildEnv::getSourceDirectory() + "/../tests/network/http/response-body.http" );
  network::protocol::http::HTTPFragment::ParseResult result = response.parse( sbuf );
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }

  if ( response.getResponseLine().getHTTPVersion().getMajor()!= 1 || response.getResponseLine().getHTTPVersion().getMinor() != 1  ) {
    std::cout << "error HTTP VERSION does not match major=" << response.getResponseLine().getHTTPVersion().getMajor()
              << " minor=" << response.getResponseLine().getHTTPVersion().getMinor() << std::endl;
    return false;
  }
  if ( !result.ok() ) {
    std::cout << "parse failed " << result.asString() << std::endl;
    return false;
  }
  if ( response.getResponseLine().getHTTPCode() != 202 ) {
    std::cout << "HTTPCode parse failure" << std::endl;
    return false;
  }
  if ( response.getBody().asString() != "All is well" ) {
    std::cout << "body parse failure (" << response.getBody().asString() << ")" << std::endl;
    return false;
  }

  std::cout << "OK" << std::endl;

  return true;
}

int main() {
  int error = 0;
  try {
    dodo::initLibrary();

    bool ok = true;
    ok = test1();

    ok = ok && test2();

    ok = ok && test3();

    ok = ok && test4();

    ok = ok && test5();

    ok = ok && test6();

    ok = ok && test7();

    error = ( ok != true );
  }
  catch ( const std::exception& e ) {
    cerr << e.what() << endl;
    error = 2;
  }
  dodo::closeLibrary();
  return error;
}