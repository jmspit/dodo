#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace std;


bool verifyBase64( const std::string &test, const std::string &base64 ) {
  dodo::common::OctetArray oa = test;
  std::cout << "test string     : " << test << std::endl;
  std::cout << "base64 string   : " << base64 << std::endl;
  std::cout << "encodeBase64    : " << oa.encodeBase64() << std::endl;
  std::cout << "encode(decode)  : " << std::string( oa.decodeBase64( oa.encodeBase64() ) ) << std::endl;
  bool ok = ( oa.encodeBase64() == base64 );
  return ok;
}

bool test1() {
  bool ok = true;
  ok = ok && verifyBase64( "Hello world", "SGVsbG8gd29ybGQ=" );
  ok = ok && verifyBase64( "This function will return the length of the data decoded or -1 on error.",
                           "VGhpcyBmdW5jdGlvbiB3aWxsIHJldHVybiB0aGUgbGVuZ3RoIG9mIHRoZSBkYXRhIGRlY29kZWQgb3IgLTEgb24gZXJyb3Iu" );
  ok = ok && verifyBase64( "The EVP encode routines provide a high level interface to base 64 encoding and decoding. Base 64 encoding converts binary data into a printable form that uses the characters A-Z, a-z, 0-9, \"+\" and \"/\" to represent the data.",
                           "VGhlIEVWUCBlbmNvZGUgcm91dGluZXMgcHJvdmlkZSBhIGhpZ2ggbGV2ZWwgaW50ZXJmYWNlIHRvIGJhc2UgNjQgZW5jb2RpbmcgYW5kIGRlY29kaW5nLiBCYXNlIDY0IGVuY29kaW5nIGNvbnZlcnRzIGJpbmFyeSBkYXRhIGludG8gYSBwcmludGFibGUgZm9ybSB0aGF0IHVzZXMgdGhlIGNoYXJhY3RlcnMgQS1aLCBhLXosIDAtOSwgIisiIGFuZCAiLyIgdG8gcmVwcmVzZW50IHRoZSBkYXRhLg==" );
  ok = ok && verifyBase64( "To maintain international peace and security, and to that end: to take effective collective measures for the prevention and removal of threats to the peace, and for the suppression of acts of aggression or other breaches of the peace, and to bring about by peaceful means, and in conformity with the principles of justice and international law, adjustment or settlement of international disputes or situations which might lead to a breach of the peace;",
                           "VG8gbWFpbnRhaW4gaW50ZXJuYXRpb25hbCBwZWFjZSBhbmQgc2VjdXJpdHksIGFuZCB0byB0aGF0IGVuZDogdG8gdGFrZSBlZmZlY3RpdmUgY29sbGVjdGl2ZSBtZWFzdXJlcyBmb3IgdGhlIHByZXZlbnRpb24gYW5kIHJlbW92YWwgb2YgdGhyZWF0cyB0byB0aGUgcGVhY2UsIGFuZCBmb3IgdGhlIHN1cHByZXNzaW9uIG9mIGFjdHMgb2YgYWdncmVzc2lvbiBvciBvdGhlciBicmVhY2hlcyBvZiB0aGUgcGVhY2UsIGFuZCB0byBicmluZyBhYm91dCBieSBwZWFjZWZ1bCBtZWFucywgYW5kIGluIGNvbmZvcm1pdHkgd2l0aCB0aGUgcHJpbmNpcGxlcyBvZiBqdXN0aWNlIGFuZCBpbnRlcm5hdGlvbmFsIGxhdywgYWRqdXN0bWVudCBvciBzZXR0bGVtZW50IG9mIGludGVybmF0aW9uYWwgZGlzcHV0ZXMgb3Igc2l0dWF0aW9ucyB3aGljaCBtaWdodCBsZWFkIHRvIGEgYnJlYWNoIG9mIHRoZSBwZWFjZTs=" );
  return ok;
}


int main() {
  int error = 0;
  bool ok = true;
  try {
    dodo::initLibrary();
    ok = ok && test1();
  }
  catch ( const std::exception& e ) {
    cerr << e.what() << endl;
    error = 2;
  }
  dodo::closeLibrary();
  if ( !error && !ok ) error = 1;
  return error;
}