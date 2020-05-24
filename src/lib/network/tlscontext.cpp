/*
 * This file is part of the dodo library (https://github.com/jmspit/dodo).
 * Copyright (c) 2019 Jan-Marten Spit.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file tlscontext.cpp
 * Implements the dodo::network::TLSContext class.
 */

#include "common/exception.hpp"
#include <network/tlscontext.hpp>

#include <string.h>

#include <openssl/ssl.h>
#include <openssl/pkcs12.h>

#include <iostream>

namespace dodo::network {

  void TLSContext::InitializeSSL() {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
  }

  void TLSContext::ShutdownSSL() {
    ERR_free_strings();
    EVP_cleanup();
  }

  TLSContext::TLSContext( const TLSContext::PeerVerification& peerverficiation,
                          const TLSVersion& tlsversion ) {
    tlsversion_ = tlsversion;
    peerverficiation_ = peerverficiation;
    passphrase_ = "";
    tlsctx_ = nullptr;
    long rc = 0;
    long ssl_options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
    tlsctx_ = SSL_CTX_new( TLS_method() );
    if ( !tlsctx_ ) throw_ExceptionObject( common::Puts() << "SSL_CTX_new failed"
                                           << common::Puts::endl() << getSSLErrors( '\n' ), this  );
    switch( tlsversion_ ) {
      case TLSVersion::tls1_1 :
        rc = SSL_CTX_set_min_proto_version( tlsctx_, TLS1_1_VERSION );
        break;
      case TLSVersion::tls1_2 :
        ssl_options = ssl_options | SSL_OP_NO_TLSv1_1;
        rc = SSL_CTX_set_min_proto_version( tlsctx_, TLS1_2_VERSION );
        break;
      case TLSVersion::tls1_3 :
        ssl_options = ssl_options | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2;
        rc = SSL_CTX_set_min_proto_version( tlsctx_, TLS1_3_VERSION );
        break;
    }
    if ( rc == 0 ) throw_ExceptionObject( common::Puts() << "SSL_CTX_set_min_proto_version failed"
                                          << common::Puts::endl() << getSSLErrors( '\n' ), this  );
    SSL_CTX_set_options( tlsctx_, ssl_options );

    SSL_CTX_set_default_passwd_cb( tlsctx_, pem_passwd_cb );
    SSL_CTX_set_default_passwd_cb_userdata( tlsctx_, this );

    if ( peerverficiation == PeerVerification::pvNone ) {
      SSL_CTX_set_verify( tlsctx_, SSL_VERIFY_NONE, nullptr );
    } else {
      SSL_CTX_set_verify( tlsctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr );
    }

    rc = SSL_CTX_set_default_verify_paths(tlsctx_);
    if ( rc == 0 ) throw_ExceptionObject( common::Puts() << "SSL_CTX_set_default_verify_paths failed"
                                          << common::Puts::endl() << getSSLErrors( '\n' ), this  );
  }

  TLSContext::~TLSContext() {
    passphrase_ = "";
    SSL_CTX_free( tlsctx_ );
  }

  int TLSContext::pem_passwd_cb( char *buf, int size, int rwflag, void *userdata ) {
    TLSContext* tlsctx = static_cast<TLSContext*>(userdata);
    if ( size > static_cast<int>( strlen( tlsctx->passphrase_.c_str() ) ) ) {
      strncpy( buf, tlsctx->passphrase_.c_str(), size );
    } else buf[0] = 0;
    buf[size-1] = 0;
    return static_cast<int>( strlen( tlsctx->passphrase_.c_str() ) );
  }

  void TLSContext::loadPEMIdentity( const std::string& certfile,
                                    const std::string& keyfile,
                                    const std::string& passphrase ) {
    passphrase_ = passphrase;
    if ( SSL_CTX_use_certificate_file( tlsctx_, certfile.c_str(), SSL_FILETYPE_PEM ) != 1 ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    if ( SSL_CTX_use_PrivateKey_file( tlsctx_, keyfile.c_str(), SSL_FILETYPE_PEM ) != 1 ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    if ( !SSL_CTX_check_private_key( tlsctx_ ) ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    passphrase_ = "";
  }

  void TLSContext::loadPKCS12( const std::string &p12file,
                               const std::string &p12passphrase ) {
    passphrase_ = "";
    PKCS12 *p12 = nullptr;
    FILE *fp = 0;
    if ( ( fp = fopen( p12file.c_str(), "rb" ) ) )
    {
      p12 = d2i_PKCS12_fp( fp, NULL );
      if ( p12 ) {
        EVP_PKEY *pkey = nullptr;
        X509 *cert = nullptr;
        STACK_OF(X509) *ca = nullptr;
        if ( PKCS12_parse( p12, p12passphrase.c_str(), &pkey, &cert, &ca) ) {

          try {

            if ( SSL_CTX_use_certificate( tlsctx_, cert )  != 1  )
              throw_ExceptionObject( common::Puts() << "cannot use certificate from '" << p12file << "'"
                                                    << common::Puts::endl() << getSSLErrors( '\n' ), this  );

            if ( SSL_CTX_use_PrivateKey( tlsctx_, pkey )  != 1  || !pkey )
              throw_ExceptionObject( common::Puts() << "cannot use private key from '" << p12file << "'"
                                                    <<  common::Puts::endl() << getSSLErrors( '\n' ), this  );

            if ( !SSL_CTX_check_private_key( tlsctx_ ) )
              throw_ExceptionObject( common::Puts() << "invalid private key in '" << p12file << "'"
                                                    << common::Puts::endl() << getSSLErrors( '\n' ), this  );

            if ( !SSL_CTX_set0_chain( tlsctx_, ca ) ) {

              throw_ExceptionObject( common::Puts() << "cannot use certificate chain from '" << p12file << "'"
                                                    << common::Puts::endl() << getSSLErrors( '\n' ), this  );
            }
            if ( cert ) X509_free( cert );
            if ( pkey ) EVP_PKEY_free( pkey );
            if ( p12 ) PKCS12_free( p12 );
            fclose( fp );
          }
          catch ( ... ) {
            if ( cert ) X509_free( cert );
            if ( pkey ) EVP_PKEY_free( pkey );
            if ( p12 ) PKCS12_free( p12 );
            fclose( fp );
            throw;
          }
        } else throw_ExceptionObject( common::Puts() << "cannot parse PKCS12 file '" << p12file << "'"
                                                     << common::Puts::endl() << getSSLErrors( '\n' ), this  );
      } else throw_ExceptionObject( common::Puts() << "cannot read PKCS12 file '" << p12file << "'"
                                                   << common::Puts::endl() << getSSLErrors( '\n' ), this  );
    } else throw_ExceptionObject( common::Puts() << "cannot open'" << p12file << "'"
                                                 << common::Puts::endl() << getSSLErrors( '\n' ), this  );
  }

  void TLSContext::setCipherList( const std::string& cipherlist ) {
    int rc = 0;
    if ( tlsversion_ == TLSVersion::tls1_3 ) {
      rc = SSL_CTX_set_ciphersuites( tlsctx_, cipherlist.c_str() );
    } else {
      rc = SSL_CTX_set_cipher_list( tlsctx_, cipherlist.c_str() );
    }
    if ( rc != 1 ) throw_ExceptionObject( common::Puts() << "invalid cipherlist '" <<
                                          cipherlist << "'", this  );
  }

  long TLSContext::setOptions( long options ) {
    return SSL_CTX_set_options( tlsctx_, options );
  }

  void TLSContext::setTrustPaths(  const std::string& cafile,
                                   const std::string& capath ) {
    const char *cafile_ptr = nullptr;
    const char *capath_ptr = nullptr;
    if ( cafile.length() > 0 ) cafile_ptr = cafile.c_str();
    if ( capath.length() > 0 ) capath_ptr = capath.c_str();
    if ( !SSL_CTX_load_verify_locations( tlsctx_, cafile_ptr, capath_ptr ) )
      throw_ExceptionObject( common::Puts() << "SSL_CTX_load_verify_locations failed"
                                            << common::Puts::endl() << getSSLErrors( '\n' ), this  );
  }

  size_t TLSContext::writeSSLErrors( std::ostream& out, char terminator ) {
    size_t count = 0;
    unsigned long error = 0;
    // https://www.openssl.org/docs/man1.0.2/man3/ERR_error_string.html buf must be at least 120 bytes
    char errbuf[200];
    while ( ( error = ERR_get_error() ) ) {
      ERR_error_string_n( error, errbuf, sizeof(errbuf) );
      out << errbuf;
      if ( terminator ) out << terminator;
      count++;
    }
    return count;
  }

  std::string TLSContext::getSSLErrors( char terminator ) {
    std::stringstream ss;
    if ( writeSSLErrors( ss, terminator ) ) {
      return ss.str();
    } else return "";
  }


}