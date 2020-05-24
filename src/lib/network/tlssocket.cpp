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
 * @file tlssocket.cpp
 * Implements the dodo::network::TLSSocket class.
 */

#include "network/tlssocket.hpp"

#include <string.h>


namespace dodo::network {

  TLSSocket::TLSSocket( int socket, TLSContext& tlscontext ) : BaseSocket( socket ), tlscontext_(tlscontext) {
  }

  TLSSocket::TLSSocket( bool blocking,
                        SocketParams params,
                        TLSContext& tlscontext ) :
                        BaseSocket( blocking, params ), tlscontext_(tlscontext) {
    ssl_ = SSL_new( tlscontext .getContext() );
    if ( !ssl_ ) throw_Exception( tlscontext.getSSLErrors( '\n' )  );
    SSL_set_fd( ssl_, socket_ );
  }

  TLSSocket::~TLSSocket() {
    SSL_free( ssl_ );
  }

  SystemError TLSSocket::connect( const Address &address ) {
    SystemError error = BaseSocket::connect( address );
    if ( error == common::SystemError::ecOK ) {
      auto rc = SSL_connect( ssl_ );
      if ( rc != 1 ) {
        switch ( SSL_get_error( ssl_, rc ) ) {
          case SSL_ERROR_NONE :             return SystemError::ecSSL_ERROR_NONE;
          case SSL_ERROR_ZERO_RETURN :      return SystemError::ecSSL_ERROR_ZERO_RETURN;
          case SSL_ERROR_WANT_READ :        return SystemError::ecSSL_ERROR_WANT_READ;
          case SSL_ERROR_WANT_WRITE :       return SystemError::ecSSL_ERROR_WANT_WRITE;
          case SSL_ERROR_WANT_CONNECT :     return SystemError::ecSSL_ERROR_WANT_CONNECT;
          case SSL_ERROR_WANT_ACCEPT :      return SystemError::ecSSL_ERROR_WANT_ACCEPT;
          case SSL_ERROR_WANT_X509_LOOKUP : return SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
          case SSL_ERROR_SSL :
          default: throw_Exception( tlscontext_.getSSLErrors( '\n' )  );
        }
      }
      return SystemError::ecOK;
    } else return error;
  }

  SystemError TLSSocket::accept() {
    auto rc = SSL_accept( ssl_ );
    if ( rc <= 0 ) {
      switch ( SSL_get_error( ssl_, rc ) ) {
        case SSL_ERROR_NONE :             return SystemError::ecSSL_ERROR_NONE;
        case SSL_ERROR_ZERO_RETURN :      return SystemError::ecSSL_ERROR_ZERO_RETURN;
        case SSL_ERROR_WANT_READ :        return SystemError::ecSSL_ERROR_WANT_READ;
        case SSL_ERROR_WANT_WRITE :       return SystemError::ecSSL_ERROR_WANT_WRITE;
        case SSL_ERROR_WANT_CONNECT :     return SystemError::ecSSL_ERROR_WANT_CONNECT;
        case SSL_ERROR_WANT_ACCEPT :      return SystemError::ecSSL_ERROR_WANT_ACCEPT;
        case SSL_ERROR_WANT_X509_LOOKUP : return SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
        case SSL_ERROR_SSL :
        default: throw_Exception( tlscontext_.getSSLErrors( '\n' )  );
      }
    }
    return SystemError::ecOK;
  }

  X509* TLSSocket::getPeerCertificate() const {
    return SSL_get_peer_certificate(ssl_);
  }

  SystemError TLSSocket::send( const void* buf, ssize_t len, bool more ) {
    auto rc = SSL_write( ssl_, buf, (int)len );
    if ( rc <= 0 ) {
      switch ( SSL_get_error( ssl_, rc ) ) {
        case SSL_ERROR_NONE :             return SystemError::ecSSL_ERROR_NONE;
        case SSL_ERROR_ZERO_RETURN :      return SystemError::ecSSL_ERROR_ZERO_RETURN;
        case SSL_ERROR_WANT_READ :        return SystemError::ecSSL_ERROR_WANT_READ;
        case SSL_ERROR_WANT_WRITE :       return SystemError::ecSSL_ERROR_WANT_WRITE;
        case SSL_ERROR_WANT_CONNECT :     return SystemError::ecSSL_ERROR_WANT_CONNECT;
        case SSL_ERROR_WANT_ACCEPT :      return SystemError::ecSSL_ERROR_WANT_ACCEPT;
        case SSL_ERROR_WANT_X509_LOOKUP : return SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
        case SSL_ERROR_SSL :
        default: throw_Exception( tlscontext_.getSSLErrors( '\n' )  );
      }
    }
    return SystemError::ecOK;
  }

  SystemError TLSSocket::receive( void* buf, ssize_t request, ssize_t &received ) {
    received = 0;
    auto rc = SSL_read( ssl_, buf, (int)request );
    if ( rc <= 0 ) {
      switch ( SSL_get_error( ssl_, (int)received ) ) {
        case SSL_ERROR_NONE :             return SystemError::ecSSL_ERROR_NONE;
        case SSL_ERROR_ZERO_RETURN :      return SystemError::ecSSL_ERROR_ZERO_RETURN;
        case SSL_ERROR_WANT_READ :        return SystemError::ecSSL_ERROR_WANT_READ;
        case SSL_ERROR_WANT_WRITE :       return SystemError::ecSSL_ERROR_WANT_WRITE;
        case SSL_ERROR_WANT_CONNECT :     return SystemError::ecSSL_ERROR_WANT_CONNECT;
        case SSL_ERROR_WANT_ACCEPT :      return SystemError::ecSSL_ERROR_WANT_ACCEPT;
        case SSL_ERROR_WANT_X509_LOOKUP : return SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
        case SSL_ERROR_SSL :
        default: throw_Exception( tlscontext_.getSSLErrors( '\n' )  );
      }
    } else received = rc;
    return SystemError::ecOK;
  }

  TLSSocket& TLSSocket::operator=( const TLSSocket& socket ) {
    socket_ = socket.socket_;
    ssl_ = socket.ssl_;
    tlscontext_ = socket.tlscontext_;
    return *this;
  }

}
