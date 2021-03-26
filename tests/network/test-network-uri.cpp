#include <iostream>
#include <dodo.hpp>

using namespace dodo;

//https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top

bool test1() {
  const std::string tst_uri = "https://www.example.com";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test2() {
  const std::string tst_uri = "https://www.example.com/";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "/" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test3() {
  const std::string tst_uri = "https://john.doe@www.example.com/";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "john.doe" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "/" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test4() {
  const std::string tst_uri = "https://john.doe@www.example.com:123/";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "john.doe" ) return false;
    if ( uri.getPort() != "123" ) return false;
    if ( uri.getPath() != "/" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test5() {
  const std::string tst_uri = "https://john.doe@www.example.com:123/forum/questions/";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "john.doe" ) return false;
    if ( uri.getPort() != "123" ) return false;
    if ( uri.getPath() != "/forum/questions/" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test6() {
  const std::string tst_uri = "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "john.doe" ) return false;
    if ( uri.getPort() != "123" ) return false;
    if ( uri.getPath() != "/forum/questions/" ) return false;
    if ( uri.getQuery() != "tag=networking&order=newest" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test7() {
  const std::string tst_uri = "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "https" ) return false;
    if ( uri.getHost() != "www.example.com" ) return false;
    if ( uri.getUserInfo() != "john.doe" ) return false;
    if ( uri.getPort() != "123" ) return false;
    if ( uri.getPath() != "/forum/questions/" ) return false;
    if ( uri.getQuery() != "tag=networking&order=newest" ) return false;
    if ( uri.getFragment() != "top" ) return false;
  } else return false;
  return true;
}

bool test8() {
  const std::string tst_uri = "ldap://[2001:db8::7]/c=GB?objectClass?one";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "ldap" ) return false;
    if ( uri.getHost() != "[2001:db8::7]" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "/c=GB" ) return false;
    if ( uri.getQuery() != "objectClass?one" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test9() {
  const std::string tst_uri = "mailto:John.Doe@example.com";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "mailto" ) return false;
    if ( uri.getHost() != "" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "John.Doe@example.com" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test10() {
  const std::string tst_uri = "news:comp.infosystems.www.servers.unix";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "news" ) return false;
    if ( uri.getHost() != "" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "comp.infosystems.www.servers.unix" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test11() {
  const std::string tst_uri = "tel:+1-816-555-1212";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "tel" ) return false;
    if ( uri.getHost() != "" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "+1-816-555-1212" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test12() {
  const std::string tst_uri = "telnet://192.0.2.16:80/";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "telnet" ) return false;
    if ( uri.getHost() != "192.0.2.16" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "80" ) return false;
    if ( uri.getPath() != "/" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

bool test13() {
  const std::string tst_uri = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";
  network::URI uri;
  size_t idxfail = 0;
  if ( uri.parse( tst_uri, idxfail ) ) {
    if ( uri.getScheme() != "urn" ) return false;
    if ( uri.getHost() != "" ) return false;
    if ( uri.getUserInfo() != "" ) return false;
    if ( uri.getPort() != "" ) return false;
    if ( uri.getPath() != "oasis:names:specification:docbook:dtd:xml:4.1.2" ) return false;
    if ( uri.getQuery() != "" ) return false;
    if ( uri.getFragment() != "" ) return false;
  } else return false;
  return true;
}

int main() {
  std::cout << BuildEnv::getDescription();
  bool ok = true;

  ok = ok && test1();
  if ( !ok ) return 1;

  ok = ok && test2();
  if ( !ok ) return 1;

  ok = ok && test3();
  if ( !ok ) return 1;

  ok = ok && test4();
  if ( !ok ) return 1;

  ok = ok && test5();
  if ( !ok ) return 1;

  ok = ok && test6();
  if ( !ok ) return 1;

  ok = ok && test6();
  if ( !ok ) return 1;

  ok = ok && test7();
  if ( !ok ) return 1;

  ok = ok && test8();
  if ( !ok ) return 1;

  ok = ok && test9();
  if ( !ok ) return 1;

  ok = ok && test10();
  if ( !ok ) return 1;

  ok = ok && test11();
  if ( !ok ) return 1;

  ok = ok && test12();
  if ( !ok ) return 1;

  ok = ok && test13();
  if ( !ok ) return 1;

  return 0;
}