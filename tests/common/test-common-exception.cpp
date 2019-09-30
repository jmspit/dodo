#include <iostream>
#include <dodo.hpp>

bool test1() {
  try {
    throw dodo::common::Exception( __FILE__, __LINE__, "exception1" );
  }
  catch ( std::exception &e ) {
    std::cerr << "caught std::exception: " << e.what() << std::endl;
    return true;
  }
  return false;
}

bool test2() {
  try {
    throw dodo::common::Exception( __FILE__, __LINE__, "exception2" );
  }
  catch ( dodo::common::Exception &e ) {
    std::cerr << "caught dodo::common::Exception: " << e.what() << std::endl;
    return true;
  }
  return false;
}

bool test3() {
  class Car : public dodo::common::DebugObject {
    public:
      Car() : color("red") {};
      virtual std::string debugDetail() const {
        std::stringstream ss;
        ss << "color: " << color << std::endl;
        return ss.str();
      };
      std::string color;
  };
  try {
    Car car;
    throw dodo::common::Exception( __FILE__, __LINE__, "exception3", &car );
  }
  catch ( dodo::common::Exception &e ) {
    std::cerr << "caught dodo::common::Exception: " << e.what() << std::endl;
    return true;
  }
  return false;
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