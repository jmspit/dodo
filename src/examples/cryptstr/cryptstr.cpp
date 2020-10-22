#include <iostream>
#include <dodo.hpp>

#include <fstream>
#include <unistd.h>

using namespace dodo;
using namespace std;

struct Options {
  common::DataCrypt::Cipher cipher = common::DataCrypt::Cipher::Default;
  bool encrypt = true;
  string key_type = "";
  string key_value = "";
};
Options options;


void printHelp() {
  cout << "usage: " << endl;
  cout << "cryptstr -e [-c cipher] -k keysource" << endl;
  cout << "cryptstr -d -k keysource" << endl;
  cout << "  -c" << endl;
  cout << "    specify the cipher to use (only for -e)" << endl;
  cout << "  -d" << endl;
  cout << "    decrypt (all newlines in the input will be ignored)" << endl;
  cout << "  -e" << endl;
  cout << "    encrypt" << endl;
  cout << "  -k" << endl;
  cout << "    key source, either file:PATH, env:ENV_VAR or val:VALUE" << endl;
  cout <<  endl;
  cout << "    Reading from file is the more secure option. Encryption requires the file to be" << endl;
  cout << "    readable only to the user." << endl;
  cout <<  endl;
  cout << "    The key may, but does not have to be a string. If -k file: is used, the data is read as octets up" << endl;
  cout << "    until the key size or EOF." << endl;
  cout <<  endl;
  cout << "cryptstr reads from std input and writes to std coutput. The input may be any octet stream, but the coutput" << endl;
  cout << "will always be a zero-terminated string." << endl;
  cout <<  endl;
  cout << "Apart from the key, the encrypted string contains all information required to decrypt it, so that" << endl;
  cout << "the encrypted string is not bound to any implementation - with the key the data will always be recoverable." << endl;
  cout <<  endl;
  cout << "The key size depends on the cipher used (octet, 8 bits, or 'byte')" << endl;
  cout << "  EVP_aes_128_gcm 16 octets" << endl;
  cout << "  EVP_aes_192_gcm 24 octets" << endl;
  cout << "  EVP_aes_256_gcm 32 octets (default)" << endl;
  cout <<  endl;
  cout << "If the size of the -k specified key is less than the above key size, the key is repeated"  << endl;
  cout << "until all the octets are filled."  << endl;
}

void printErrorHelp( const std::string &msg ) {
  cerr << "cannot run, please " << msg << endl;
  printHelp();
}

bool getKey() {
  if (  options.key_type == "env" ) {
    char *env = getenv(  options.key_value.c_str() );
    if ( env ) options.key_value = env; else {
      cerr << "unable to read environment variable '" << options.key_value.c_str() << "'" << endl;
      exit(EXIT_FAILURE);
    }
  } else if (  options.key_type == "file" ) {
    ifstream ifs;
    string file = options.key_value;
    ifs.open( file, ios::in );
    if ( ifs.good() ) {
      ifs >> options.key_value;
    } else {
      printErrorHelp( "make sure file '" + file + "' can be read" );
      exit(EXIT_FAILURE);
    }
  }
  return options.key_value != "";
}

bool parseArgs( int argc, char* argv[] ) {
  int opt;
  vector<string> kv;
  while ( ( opt = getopt( argc, argv, "edc:k:" ) ) != -1 ) {
    switch ( opt ) {
    case 'c':
      options.cipher = dodo::common::DataCrypt::string2Cipher( optarg );
      if ( options.cipher == dodo::common::DataCrypt::Cipher::Invalid ) {
        printHelp();
        exit(EXIT_FAILURE);
      }
      break;
    case 'e':
      options.encrypt = true;
      break;
    case 'd':
      options.encrypt = false;
      break;
    case 'h':
      printHelp();
      exit(EXIT_SUCCESS);
      break;
    case 'k':
      kv = common::split( optarg, ':' );
      if ( kv.size() == 2 &&
           ( kv[0] == "file" || kv[0]=="env" || kv[0]=="val" ) &&
           kv[1].length() > 0 ) {
        options.key_type = kv[0];
        options.key_value = kv[1];
        return getKey();
      } else {
        printErrorHelp( "specify a valid keysource" );
        exit(EXIT_FAILURE);
      }
      break;
    default: /* '?' */
      printHelp();
      exit(EXIT_FAILURE);
    }
  }
  if ( options.key_value.length() == 0 ) {
    printErrorHelp( "specify a keysource" );
    return false;
  }
  return getKey();
}

bool encrypt( std::istream& in, std::ostream& out ) {
  stringstream foo;
  do {
    char c;
    in.get(c);
    if ( in.good() ) foo << c;
  } while ( in.good() );
  in.clear();
  in.ignore(std::numeric_limits<std::streamsize>::max());
  common::OctetArray oa = foo.str();
  string key = options.key_value;
  std::string encrypted;
  dodo::common::DataCrypt::encrypt( options.cipher,
                                    key,
                                    oa,
                                    encrypted );
  cout << encrypted << endl;
  return true;
}

bool decrypt( std::istream& in, std::ostream& out ) {
  stringstream foo;
  do {
    char c;
    in.get(c);
    if ( in.good() ) foo << c;
  } while ( in.good() );
  in.clear();
  in.ignore(std::numeric_limits<std::streamsize>::max());
  common::OctetArray oa;
  string key = options.key_value;
  int rc = dodo::common::DataCrypt::decrypt( key, foo.str(), oa );
  if ( rc == 0 ) cout << oa.asString();
  else if ( rc == 1 )  {
    cerr << "not a valid crypt string" << endl;
    return false;
  } else {
    cerr << "decryption failure" << endl;
    return false;
  }
  return true;
}


int main( int argc, char* argv[] ) {
  int rc = 0;
  try {
    initLibrary();
    if ( parseArgs( argc, argv ) ) {
      if ( options.encrypt ) {
        rc = ( encrypt( cin, cout ) == false );
      } else {
        rc = ( decrypt( cin, cout ) == false );
      }
    }
    rc = 0;
  }
  catch ( const std::exception &e ) {
    cerr << e.what() << endl;
    rc = 1;
  }
  closeLibrary();
  return rc;
}