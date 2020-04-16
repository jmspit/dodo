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
 * @file sslcontext.cpp
 * Implements the dodo::network::TLSContext class.
 */

#include "common/exception.hpp"
#include <network/tlscontext.hpp>

#include <string.h>

#include <openssl/ssl.h>

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

  TLSContext::TLSContext( const TLSVersion& tlsversion ) {
    tlsversion_ = tlsversion;
    passphrase_ = "";
    sslctx_ = nullptr;
    long ssl_options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;

    switch( tlsversion_ ) {
      case TLSVersion::tls1_1 :
        sslctx_ = SSL_CTX_new( TLS_method() );
        SSL_CTX_set_min_proto_version( sslctx_, TLS1_1_VERSION );
        break;
      case TLSVersion::tls1_2 :
        sslctx_ = SSL_CTX_new( TLS_method() );
        ssl_options = ssl_options | SSL_OP_NO_TLSv1_1;
        SSL_CTX_set_min_proto_version( sslctx_, TLS1_2_VERSION );
        break;
      case TLSVersion::tls1_3 :
        sslctx_ = SSL_CTX_new( TLS_method() );
        ssl_options = ssl_options | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2;
        SSL_CTX_set_min_proto_version( sslctx_, TLS1_3_VERSION );
        break;
    }
    SSL_CTX_set_options( sslctx_, ssl_options );
  }

  TLSContext::~TLSContext() {
    SSL_CTX_free( sslctx_ );
  }

  void TLSContext::loadCertificates( const std::string& certfile, const std::string& keyfile, const std::string passphrase ) {
    if ( SSL_CTX_use_certificate_file( sslctx_, certfile.c_str(), SSL_FILETYPE_PEM ) <= 0 ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    if ( SSL_CTX_use_PrivateKey_file( sslctx_, keyfile.c_str(), SSL_FILETYPE_PEM ) <= 0 ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    if ( !SSL_CTX_check_private_key( sslctx_ ) ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    passphrase_ = passphrase;
  }

  void TLSContext::setCipherList( const std::string& cipherlist ) {
    int rc = SSL_CTX_set_cipher_list( sslctx_, cipherlist.c_str() );
    if ( rc != 1 ) throw_ExceptionObject( common::Puts() << "no valid ciphers in cipherlist '" <<
                                          cipherlist << "'", this  );
  }

  long TLSContext::setOptions( long options ) {
    return SSL_CTX_set_options( sslctx_, options );
  }

  size_t TLSContext::writeSSLErrors( std::ostream& out, char terminator ) {
    size_t count = 0;
    unsigned long error = 0;
    // https://www.openssl.org/docs/man1.0.2/man3/ERR_error_string.html buf must be at least 120 bytes
    char errbuf[120];
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