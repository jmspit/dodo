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

#include <cstring>

#include "common/logger.hpp"
#include "common/util.hpp"
#include "network/tlssocket.hpp"
#include "network/x509cert.hpp"

namespace dodo::network {

  TLSSocket::TLSSocket( int socket,
                        TLSContext& tlscontext,
                        const X509Common::SAN& peer_name ) :

                        BaseSocket( socket ), tlscontext_(tlscontext), peer_name_(peer_name) {
    ssl_ = nullptr;
  }

  TLSSocket::TLSSocket( bool blocking,
                        SocketParams params,
                        TLSContext& tlscontext,
                        const X509Common::SAN& peer_name ) :
                        BaseSocket( blocking, params ), tlscontext_(tlscontext), peer_name_(peer_name) {
    ssl_ = SSL_new( tlscontext .getContext() );
    if ( !ssl_ ) throw_Exception( common::getSSLErrors( '\n' )  );
    SSL_set_fd( ssl_, socket_ );
    if ( tlscontext.isSNIEnabled() ) {
      SSL_ctrl( ssl_, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)peer_name_.san_name.c_str());
    }
  }

  TLSSocket::~TLSSocket() {
    if (ssl_ ) SSL_free( ssl_ );
  }

  common::SystemError TLSSocket::connect( const Address &address ) {
    common::SystemError error = BaseSocket::connect( address );
    if ( error == common::SystemError::ecOK ) {
      auto rc = SSL_set_fd( ssl_, socket_ );
      if ( rc != 1 ) {
        auto ssl_error_code = SSL_get_error( ssl_, rc );
        throw_Exception( ssl_error_code << " " << common::getSSLErrors( '\n' )  );
      }
      rc = SSL_connect( ssl_ );
      if ( rc != 1 ) {
        auto ssl_error_code = SSL_get_error( ssl_, rc );
        switch ( ssl_error_code ) {
          case SSL_ERROR_NONE :             return common::SystemError::ecSSL_ERROR_NONE;
          case SSL_ERROR_ZERO_RETURN :      return common::SystemError::ecSSL_ERROR_ZERO_RETURN;
          case SSL_ERROR_WANT_READ :        return common::SystemError::ecSSL_ERROR_WANT_READ;
          case SSL_ERROR_WANT_WRITE :       return common::SystemError::ecSSL_ERROR_WANT_WRITE;
          case SSL_ERROR_WANT_CONNECT :     return common::SystemError::ecSSL_ERROR_WANT_CONNECT;
          case SSL_ERROR_WANT_ACCEPT :      return common::SystemError::ecSSL_ERROR_WANT_ACCEPT;
          case SSL_ERROR_WANT_X509_LOOKUP : return common::SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
          case SSL_ERROR_SSL :
          default: throw_Exception( ssl_error_code << " " << common::getSSLErrors( '\n' )  );
        }
      }
      if ( tlscontext_.getPeerVerification() == TLSContext::PeerVerification::pvVerifyFQDN )  {
        if ( X509Certificate::verifySAN( getPeerCertificate(), { network::X509Common::SANType::stDNS, peer_name_.san_name }, tlscontext_.isAllowSANWildcards() ) ) {
          return common::SystemError::ecOK;
        } else return common::SystemError::ecSSL_ERROR_PEERVERIFICATION;
      } else return common::SystemError::ecOK;
    } else return error;
  }

  TLSSocket* TLSSocket::accept() {
    auto rc = SSL_accept( ssl_ );
    if ( rc <= 0 ) {
      auto ssl_error_code = SSL_get_error( ssl_, rc );
      log_Error( ssl_error_code << " " << common::getSSLErrors( '\n' )  );
      return nullptr;
    } else {
      TLSSocket* ret = new TLSSocket( rc, tlscontext_, { X509Common::SANType::stDNS, "" } );
      return ret;
    }
    return nullptr;
  }

  X509* TLSSocket::getPeerCertificate() const {
    return SSL_get_peer_certificate(ssl_);
  }

  common::SystemError TLSSocket::send( const void* buf, ssize_t len, bool more ) {
    auto rc = SSL_write( ssl_, buf, (int)len );
    if ( rc <= 0 ) {
      switch ( SSL_get_error( ssl_, rc ) ) {
        case SSL_ERROR_NONE :             return common::SystemError::ecSSL_ERROR_NONE;
        case SSL_ERROR_ZERO_RETURN :      return common::SystemError::ecSSL_ERROR_ZERO_RETURN;
        case SSL_ERROR_WANT_READ :        return common::SystemError::ecSSL_ERROR_WANT_READ;
        case SSL_ERROR_WANT_WRITE :       return common::SystemError::ecSSL_ERROR_WANT_WRITE;
        case SSL_ERROR_WANT_CONNECT :     return common::SystemError::ecSSL_ERROR_WANT_CONNECT;
        case SSL_ERROR_WANT_ACCEPT :      return common::SystemError::ecSSL_ERROR_WANT_ACCEPT;
        case SSL_ERROR_WANT_X509_LOOKUP : return common::SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
        case SSL_ERROR_SSL :
        default: throw_Exception( common::getSSLErrors( '\n' )  );
      }
    }
    return common::SystemError::ecOK;
  }

  common::SystemError TLSSocket::receive( void* buf, ssize_t request, ssize_t &received ) {
    received = 0;
    auto rc = SSL_read( ssl_, buf, (int)request );
    if ( rc <= 0 ) {
      auto sge = SSL_get_error( ssl_, (int)rc );
      switch ( sge ) {
        case SSL_ERROR_NONE :             return common::SystemError::ecSSL_ERROR_NONE;
        case SSL_ERROR_ZERO_RETURN :      return common::SystemError::ecSSL_ERROR_ZERO_RETURN;
        case SSL_ERROR_WANT_READ :        return common::SystemError::ecSSL_ERROR_WANT_READ;
        case SSL_ERROR_WANT_WRITE :       return common::SystemError::ecSSL_ERROR_WANT_WRITE;
        case SSL_ERROR_WANT_CONNECT :     return common::SystemError::ecSSL_ERROR_WANT_CONNECT;
        case SSL_ERROR_WANT_ACCEPT :      return common::SystemError::ecSSL_ERROR_WANT_ACCEPT;
        case SSL_ERROR_WANT_X509_LOOKUP : return common::SystemError::ecSSL_ERROR_WANT_X509_LOOKUP;
        case SSL_ERROR_SYSCALL:           return common::SystemError::ecSSL_ERROR_SYSCALL;
        case SSL_ERROR_SSL :
        default: throw_Exception( "rc=" << rc << " SSL_get_error=" << sge << " " << common::getSSLErrors( '\n' )  );
      }
    } else received = rc;
    return common::SystemError::ecOK;
  }

}
