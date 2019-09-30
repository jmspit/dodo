#include <iostream>
#include <dodo.hpp>

using namespace dodo;

bool test1() {
  std::string s = common::Puts() << "pi=" << common::Puts::setprecision(2) << 3.143;
  std::cout << s << std::endl;
  return s == "pi=3.14";
}

bool test2() {
  std::string s = common::Puts() << '2' << "+" << "2=" << 4;
  std::cout << s << std::endl;
  return s == "2+2=4";
}

bool test3() {
  std::string s = common::Puts() << "address=" << common::Puts::hex() << 0xFEFEFE;
  std::cout << s << std::endl;
  return s == "address=fefefe";
}

bool test4() {
  std::string s = common::Puts() << "unsigned long max=" <<
    common::Puts::dec() << 0xFFFFFFFFFFFFFFFFULL;
  std::cout << s << std::endl;
  return s == "unsigned long max=18446744073709551615";
}

//bool test2() {
  //common::SystemError e;
  //e = EPERM;
  //return e == common::SystemError::ecEPERM;
//}

//bool test3() {
  //common::SystemError e(common::SystemError::ecENOENT);
  //std::stringstream ss;
  //ss << e.asString();
  //std::cout << ss.str() << std::endl;
  //return ss.str() == "No such file or directory (2)";
//}

int main() {
  std::cout << dodo::BuildEnv::getDescription();
  bool ok = true;
  ok = ok && test1();
  if ( !ok ) return 1;

  ok = ok && test2();
  if ( !ok ) return 1;

  ok = ok && test3();
  if ( !ok ) return 1;

  ok = ok && test4();
  if ( !ok ) return 1;

  return 0;
}