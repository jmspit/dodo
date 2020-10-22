#include <iostream>
#include <dodo.hpp>

#include <iostream>
#include <common/unittest.hpp>

using namespace dodo;

class ExceptionTest : public common::UnitTest {
  public:
    ExceptionTest( const string &name, const string &description, ostream *out ) :
      UnitTest( name, description, out ) {};
  protected:
    virtual void doRun();

    bool test1();
    bool test2();
    bool test3();
};

void ExceptionTest::doRun() {
  test1();
  test2();
  test3();
}

bool ExceptionTest::test1() {
  bool ok = false;
  try {
    throw dodo::common::Exception( __FILE__, __LINE__, "exception1" );
  }
  catch ( std::exception &e ) {
    std::cerr << "caught std::exception: " << e.what() << std::endl;
    ok = true;
  }
  return writeSubTestResult( "test Exception", "test catch of std::exception", ok );
}

bool ExceptionTest::test2() {
  bool ok = false;
  try {
    throw dodo::common::Exception( __FILE__, __LINE__, "exception2" );
  }
  catch ( dodo::common::Exception &e ) {
    std::cerr << "caught dodo::common::Exception: " << e.what() << std::endl;
    ok = true;
  }
  return writeSubTestResult( "test Exception", "test catch of dodo::common::Exception", ok );
}

bool ExceptionTest::test3() {
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
  bool ok = false;
  try {
    Car car;
    throw dodo::common::Exception( __FILE__, __LINE__, "exception3", &car );
  }
  catch ( dodo::common::Exception &e ) {
    std::cerr << "caught dodo::common::Exception: " << e.what();
    ok = true;
  }
  return writeSubTestResult( "test Exception", "test catch of dodo::common::DebugObject", ok );
}


int main() {
  int error = 0;
  bool ok = true;
  try {
    dodo::initLibrary();
    ExceptionTest test( "common::Exception tests", "Testing Exception class", &cout );
    return test.run() == false;
  }
  catch ( const std::exception& e ) {
    cerr << e.what() << endl;
    error = 2;
  }
  dodo::closeLibrary();
  if ( !error && !ok ) error = 1;
  return error;
}