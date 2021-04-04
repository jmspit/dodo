#include <iostream>
#include <dodo.hpp>
#include <common/unittest.hpp>

using namespace dodo;
using namespace std;

class DataCryptTest : public common::UnitTest {
  public:
    DataCryptTest( const string &name, const string &description, ostream *out ) :
      UnitTest( name, description, out ) {};
  protected:
    virtual void doRun();

    bool test1();
    bool test2();
    bool test3();
    bool test4();

    bool verifyEncryption( dodo::common::DataCrypt::Cipher cipher, const std::string &key, const std::string &test );

};

void DataCryptTest::doRun() {
  test1();
  test2();
  test3();
  test4();
}

bool DataCryptTest::verifyEncryption( dodo::common::DataCrypt::Cipher cipher, const std::string &key, const std::string &test ) {
  std::cout << "key             : " << key << std::endl;
  std::cout << "test string     : " << test << std::endl;

  std::string encrypted;
  dodo::common::DataCrypt::encrypt( cipher,
                                    key,
                                    test,
                                    encrypted );
  cout << encrypted << endl;

  dodo::common::Bytes dest;
  dodo::common::DataCrypt::decrypt( key, encrypted, dest );

  return dest.asString() == test;
}

bool DataCryptTest::test1() {
  bool ok = true;
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_128_gcm, "secret", "This has been encrypted and decrypted." );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_192_gcm, "secret", "This has been encrypted and decrypted." );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_256_gcm, "secret", "This has been encrypted and decrypted." );
  return writeSubTestResult( "en/decryption", "test all supported ciphers", ok );
}

bool DataCryptTest::test2() {
  int rc = 0;
  std::string key = "secret";
  // this has 1 char damaged and should not decrypt
  std::string test = "ENC[cipher:EVP_aes_128_gcm,data:y0aXKeVMglWTOJ8c817EkLTiGJ9HdvplZsI8VW2nBZLdB9yiMVo=,iv:rMp6eCmQpZdCRHbr0OLEDQ==,tag:HwkCKBF1MS5NTz6VLDg67w==";
  dodo::common::Bytes dest;
  rc = dodo::common::DataCrypt::decrypt( key, test, dest );
  return writeSubTestResult( "en/decryption", "test detection of invalid cryptstring", rc == 1 );
}

bool DataCryptTest::test3() {
  int rc = 0;
  // the test string has valid format but we damage the key
  std::string test = "ENC[cipher:EVP_aes_128_gcm,data:y0aXKeVMglWTOJ8c817EkLTiGJ9HdvplZsI8VW2nBZLdB9yiMVo=,iv:rMp6eCmQpZdCRHbr0OLEDQ==,tag:HwkCKBF1MS5NTz6VLDg67w==]";
  std::string key = "sekreet";
  dodo::common::Bytes dest;
  rc = dodo::common::DataCrypt::decrypt( key, test, dest );
  return writeSubTestResult( "en/decryption", "test detection of decryption failure (invalid key)", rc == 2 );
}

bool DataCryptTest::test4() {
  int rc = 0;
  // the test string has valid format but we damage the key
  std::string test = "[cipher:EVP_aes_128_gcm,data:y0aXKeVMglWTOJ8c817EkLTiGJ9HdvplZsI8VW2nBZLdB9yiMVo=,iv:rMp6eCmQpZdCRHbr0OLEDQ==,tag:HwkCKBF1MS5NTz6VLDg67w==]";
  std::string key = "secret";
  dodo::common::Bytes dest;
  rc = dodo::common::DataCrypt::decrypt( key, test, dest );
  return writeSubTestResult( "en/decryption", "test detection of decryption failure (invalid format)", rc == 1 );
}

int main() {
  int error = 0;
  bool ok = true;
  try {
    dodo::initLibrary();
    DataCryptTest test( "common::DataCrypt tests", "Testing DataCrypt class", &cout );
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