/*
 * This file is part of the arca library (https://github.com/jmspit/arca).
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
 * Implements the arca::network::SSLContext class.
 */

#include "common/exception.hpp"
#include <network/sslcontext.hpp>

#include <string.h>

namespace dodo::network {

  void SSLContext::InitializeSSL() {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
  }

  void SSLContext::ShutdownSSL() {
    ERR_free_strings();
    EVP_cleanup();
  }

  SSLContext::SSLContext() {
    sslctx_ = SSL_CTX_new( SSLv23_method() );
  }

  SSLContext::~SSLContext() {
    SSL_CTX_free( sslctx_ );
  }

  common::SystemError SSLContext::loadCertificates( const string& certfile, const string& keyfile ) {
    if ( SSL_CTX_use_certificate_file( sslctx_, certfile.c_str(), SSL_FILETYPE_PEM ) <= 0 ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    if ( SSL_CTX_use_PrivateKey_file( sslctx_, keyfile.c_str(), SSL_FILETYPE_PEM ) <= 0 ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    if ( !SSL_CTX_check_private_key( sslctx_ ) ) {
      throw_ExceptionObject( getSSLErrors( '\n' ), this  );
    }
    return common::SystemError::ecOK;
  }

  void SSLContext::setCipherList( const string& cipherlist ) {
    int rc = SSL_CTX_set_cipher_list( sslctx_, cipherlist.c_str() );
    if ( rc != 1 ) throw_ExceptionObject( common::Puts() << "no valid ciphers in cipherlist '" <<
                                          cipherlist << "'", this  );
  }

  long SSLContext::setOptions( long options ) {
    return SSL_CTX_set_options( sslctx_, options );
  }

  size_t SSLContext::writeSSLErrors( ostream& out, char terminator ) {
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

  string SSLContext::getSSLErrors( char terminator ) {
    stringstream ss;
    if ( writeSSLErrors( ss, terminator ) ) {
      return ss.str();
    } else return "";
  }


}