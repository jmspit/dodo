#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace std;


bool verifyEncryption( dodo::common::DataCrypt::Cipher cipher, const std::string &key, const std::string &test ) {
  std::cout << "key             : " << key << std::endl;
  std::cout << "test string     : " << test << std::endl;

  std::string encrypted;
  dodo::common::DataCrypt::encrypt( cipher,
                                    key,
                                    test,
                                    encrypted );
  cout << encrypted << endl;

  dodo::common::OctetArray dest;
  dodo::common::DataCrypt::decrypt( key, encrypted, dest );

  return std::string(dest) == test;
}

bool test1() {
  bool ok = true;
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_128_gcm, "secret", "This has been encrypted and decrypted." );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_192_gcm, "secret", "This has been encrypted and decrypted." );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_256_gcm, "secret", "This has been encrypted and decrypted." );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_128_gcm, "skjsja72^2ka", "The HAVEGE (HArdware Volatile Entropy Gathering and Expansion) algorithum harvests the indirect effects of hardware events on hidden processor state (caches, branch predictors, memory translation tables, etc) to generate a random sequence. " );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_192_gcm, "skjsja72^2ka", "The HAVEGE (HArdware Volatile Entropy Gathering and Expansion) algorithum harvests the indirect effects of hardware events on hidden processor state (caches, branch predictors, memory translation tables, etc) to generate a random sequence. " );
  ok = ok && verifyEncryption( dodo::common::DataCrypt::Cipher::EVP_aes_256_gcm, "skjsja72^2ka", "The HAVEGE (HArdware Volatile Entropy Gathering and Expansion) algorithum harvests the indirect effects of hardware events on hidden processor state (caches, branch predictors, memory translation tables, etc) to generate a random sequence. " );
  return ok;
}

bool test2() {
  std::string key = "secret";
  // this has 1 char damaged and should not decrypt
  std::string test = "ENC[cipher:EVP_aes_256_gcm,data:IPqtWp8sCdHdJi6le/3jZWUvcNJlcLGoT9gUPnfKYiUlP6AHSvc=,iv:FaIHZmrTdIF6qXx0WUOARw==,tag:k6z30ip5WGThqhonCyAI2w==]";
  dodo::common::OctetArray dest;
  bool ok = ! dodo::common::DataCrypt::decrypt( key, test, dest );

  // the test string is valid but we damage the key
  test = "ENC[cipher:EVP_aes_128_gcm,data:y0aXKeVMglWTOJ8c817EkLTiGJ9HdvplZsI8VW2nBZLdB9yiMVo=,iv:rMp6eCmQpZdCRHbr0OLEDQ==,tag:HwkCKBF1MS5NTz6VLDg67w==]";
  key = "sekreet";
  ok = ok && ! dodo::common::DataCrypt::decrypt( key, test, dest );
  return ok;
}

int main() {
  int error = 0;
  bool ok = true;
  try {
    dodo::initLibrary();
    ok = ok && test1();
    ok = ok && test2();
    //ok = ok && test3();

  }
  catch ( const std::exception& e ) {
    cerr << e.what() << endl;
    error = 2;
  }
  dodo::closeLibrary();
  if ( !error && !ok ) error = 1;
  return error;
}