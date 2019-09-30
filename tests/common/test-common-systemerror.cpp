#include <iostream>
#include <dodo.hpp>

bool test1() {
  dodo::common::SystemError e( ENOENT );
  return e == dodo::common::SystemError::ecENOENT;
}

bool test2() {
  dodo::common::SystemError e;
  e = EPERM;
  return e == dodo::common::SystemError::ecEPERM;
}

bool test3() {
  dodo::common::SystemError e(dodo::common::SystemError::ecENOENT);
  std::stringstream ss;
  ss << e.asString();
  std::cout << ss.str() << std::endl;
  return ss.str() == "No such file or directory (2)";
}

int main() {
  std::cout << dodo::BuildEnv::getDescription();
  bool ok = true;
  ok = ok && test1();
  if ( !ok ) return 1;

  ok = ok && test2();
  if ( !ok ) return 1;

  ok = ok && test3();
  if ( !ok ) return 1;

  return 0;
}