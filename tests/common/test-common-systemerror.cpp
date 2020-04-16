#include <iostream>
#include <dodo.hpp>

#include <common/unittest.hpp>

using namespace dodo;

class SystemErrorTest : public common::UnitTest {
  public:
    SystemErrorTest( const string &name, const string &description, ostream *out ) :
      UnitTest( name, description, out ) {};
  protected:
    virtual void doRun();

    bool test1();
    bool test2();
    bool test3();
};

void SystemErrorTest::doRun() {
  test1();
  test2();
  test3();
}

bool SystemErrorTest::test1() {
  dodo::common::SystemError e( ENOENT );
  return writeSubTestResult( "test SystemErrorTest",
                             "test assignment and equality",
                             e == dodo::common::SystemError::ecENOENT );
}

bool SystemErrorTest::test2() {
  dodo::common::SystemError e;
  e = EPERM;
  return writeSubTestResult( "test SystemErrorTest",
                             "test assignment and equality",
                             e == dodo::common::SystemError::ecEPERM );
}

bool SystemErrorTest::test3() {
  dodo::common::SystemError e(dodo::common::SystemError::ecENOENT);
  std::stringstream ss;
  ss << e.asString();
  std::cout << ss.str() << std::endl;
  return writeSubTestResult( "test SystemErrorTest",
                             "test init as string conversion",
                             ss.str() == "No such file or directory (2)" );
}

int main() {
  SystemErrorTest test( "common::SystemError tests", "Testing SystemError class", &cout );
  return test.run() == false;
}