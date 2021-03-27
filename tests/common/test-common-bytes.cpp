#include <iostream>
#include <dodo.hpp>
#include <common/unittest.hpp>

using namespace dodo;
using namespace std;

class BytesTest : public common::UnitTest {
  public:
    BytesTest( const string &name, const string &description, ostream *out ) :
      UnitTest( name, description, out ) {};
  protected:
    virtual void doRun();

    bool test1();
    bool test2();
    bool test3();
    bool test4();
    bool test5();
    bool test6();

    bool verifyBase64( const std::string &test, const std::string &base64 );
};

void BytesTest::doRun() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}


bool BytesTest::verifyBase64( const std::string &test, const std::string &base64 ) {
  common::Bytes oa = test;
  //std::cout << "test string     : " << test << std::endl;
  //std::cout << "base64 string   : " << base64 << std::endl;
  //std::cout << "encodeBase64    : " << oa.encodeBase64() << std::endl;
  //std::cout << "encode(decode)  : " << oa.decodeBase64( oa.encodeBase64() ).asString() << std::endl;
  bool ok = ( oa.encodeBase64() == base64 );
  return ok;
}

bool BytesTest::test1() {
  bool ok = true;
  ok = ok && verifyBase64( "Hello world", "SGVsbG8gd29ybGQ=" );
  ok = ok && verifyBase64( "This function will return the length of the data decoded or -1 on error.",
                           "VGhpcyBmdW5jdGlvbiB3aWxsIHJldHVybiB0aGUgbGVuZ3RoIG9mIHRoZSBkYXRhIGRlY29kZWQgb3IgLTEgb24gZXJyb3Iu" );
  ok = ok && verifyBase64( "The EVP encode routines provide a high level interface to base 64 encoding and decoding. Base 64 encoding converts binary data into a printable form that uses the characters A-Z, a-z, 0-9, \"+\" and \"/\" to represent the data.",
                           "VGhlIEVWUCBlbmNvZGUgcm91dGluZXMgcHJvdmlkZSBhIGhpZ2ggbGV2ZWwgaW50ZXJmYWNlIHRvIGJhc2UgNjQgZW5jb2RpbmcgYW5kIGRlY29kaW5nLiBCYXNlIDY0IGVuY29kaW5nIGNvbnZlcnRzIGJpbmFyeSBkYXRhIGludG8gYSBwcmludGFibGUgZm9ybSB0aGF0IHVzZXMgdGhlIGNoYXJhY3RlcnMgQS1aLCBhLXosIDAtOSwgIisiIGFuZCAiLyIgdG8gcmVwcmVzZW50IHRoZSBkYXRhLg==" );
  ok = ok && verifyBase64( "To maintain international peace and security, and to that end: to take effective collective measures for the prevention and removal of threats to the peace, and for the suppression of acts of aggression or other breaches of the peace, and to bring about by peaceful means, and in conformity with the principles of justice and international law, adjustment or settlement of international disputes or situations which might lead to a breach of the peace;",
                           "VG8gbWFpbnRhaW4gaW50ZXJuYXRpb25hbCBwZWFjZSBhbmQgc2VjdXJpdHksIGFuZCB0byB0aGF0IGVuZDogdG8gdGFrZSBlZmZlY3RpdmUgY29sbGVjdGl2ZSBtZWFzdXJlcyBmb3IgdGhlIHByZXZlbnRpb24gYW5kIHJlbW92YWwgb2YgdGhyZWF0cyB0byB0aGUgcGVhY2UsIGFuZCBmb3IgdGhlIHN1cHByZXNzaW9uIG9mIGFjdHMgb2YgYWdncmVzc2lvbiBvciBvdGhlciBicmVhY2hlcyBvZiB0aGUgcGVhY2UsIGFuZCB0byBicmluZyBhYm91dCBieSBwZWFjZWZ1bCBtZWFucywgYW5kIGluIGNvbmZvcm1pdHkgd2l0aCB0aGUgcHJpbmNpcGxlcyBvZiBqdXN0aWNlIGFuZCBpbnRlcm5hdGlvbmFsIGxhdywgYWRqdXN0bWVudCBvciBzZXR0bGVtZW50IG9mIGludGVybmF0aW9uYWwgZGlzcHV0ZXMgb3Igc2l0dWF0aW9ucyB3aGljaCBtaWdodCBsZWFkIHRvIGEgYnJlYWNoIG9mIHRoZSBwZWFjZTs=" );
  return writeSubTestResult( "test base64",
                             "test BytesTest encode/decode base64",
                             ok );
}

bool BytesTest::test2() {
  common::Bytes o1 = { "CONNECTED\r\n" };
  common::Bytes o2 = { "CONNECTED" };
  size_t octets;
  common::Bytes::MatchType mt = o1.match( o2, 0, octets );
  return ( mt == common::Bytes::MatchType::Contains && octets == o2.getSize() );
  return writeSubTestResult( "test comparison/match ",
                             "test common::Bytes::MatchType::Contains",
                             mt == common::Bytes::MatchType::Contains && octets == o2.getSize() );
}

bool BytesTest::test3() {
  common::Bytes o1 = { "CONN" };
  common::Bytes o2 = { "CONNECTED" };
  size_t octets;
  common::Bytes::MatchType mt = o1.match( o2, 0, octets );
  return writeSubTestResult( "test comparison/match ",
                             "test common::Bytes::MatchType::Contained",
                             mt == common::Bytes::MatchType::Contained && octets == o1.getSize() );
}

bool BytesTest::test4() {
  common::Bytes o1 = { "CONNECTED" };
  common::Bytes o2 = { "CONNECTED" };
  size_t octets;
  common::Bytes::MatchType mt = o1.match( o2, 0, octets );
  return writeSubTestResult( "test comparison/match ",
                             "test common::Bytes::MatchType::Contained",
                             mt == common::Bytes::MatchType::Full && octets == o2.getSize() );
}

bool BytesTest::test5() {
  common::Bytes o1 = { "CONNECTED" };
  common::Bytes o2 = { "CONNECTER" };
  size_t octets;
  common::Bytes::MatchType mt = o1.match( o2, 0, octets );
  return writeSubTestResult( "test comparison/match ",
                             "test common::Bytes::MatchType::Contained",
                             mt == common::Bytes::MatchType::Mismatch && octets == 0 );
}

bool BytesTest::test6() {
  common::Bytes o1 = { "This" };
  common::Bytes o2 = { "This function will return the length of the data decoded or -1 on error" };
  o1.append( { " function" } );
  o1.append( { " will" } );
  o1.append( { " return" } );
  o1.append( { " the" } );
  o1.append( { " length" } );
  o1.append( { " of" } );
  o1.append( { " the" } );
  o1.append( { " data" } );
  o1.append( { " decoded" } );
  o1.append( { " or" } );
  o1.append( { " -1" } );
  o1.append( { " on" } );
  o1.append( { " error." } );
  size_t octets;
  common::Bytes::MatchType mt = o1.match( o2, 0, octets );
  return ( mt == common::Bytes::MatchType::Full && octets == o2.getSize() );
}


int main() {
  int error = 0;
  try {
    dodo::initLibrary();
    BytesTest test( "common::BytesTest tests", "Testing BytesTest class", &cout );
    error = ( test.run() == false );
  }
  catch ( const std::exception& e ) {
    cerr << e.what() << endl;
    error = 2;
  }
  dodo::closeLibrary();
  return error;
}