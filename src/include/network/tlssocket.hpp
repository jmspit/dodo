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
 * @file tlssocket.hpp
 * Defines the dodo::network::TLSSocket class.
 */

#ifndef network_tlssocket_hpp
#define network_tlssocket_hpp

#include <openssl/ssl.h>

#include "network/socket.hpp"
#include "network/socketparams.hpp"
#include "network/tlscontext.hpp"
#include "network/x509cert.hpp"

namespace dodo::network {

  //using namespace std;

  /**
   * Socket for TLS encrypted traffic between trusted endpoints.
   *
   * @ref developer_networking
   */
  class TLSSocket : public BaseSocket {
    public:

      /**
       * Construct from existing socket file descriptor.
       * @param socket The socket file descriptor.
       * @param tlscontext The TLSContext to apply.
       * @param peer_name The peer name to validate against the peer certificate CN and SubjectAltNames. Only used
       *                  when the tlscontext has pvVerifyFQDN set (and possibly when pvCustom is set).
       */
      TLSSocket( int socket, TLSContext& tlscontext, const X509Common::SAN &peer_name );

      /**
       * Construct from scratch.
       * @param blocking If true, operate in blocking mode.
       * @param params The SocketParams to apply.
       * @param tlscontext The TLSContext to apply.
       * @param peer_name The peer name to validate against the peer certificate CN and SubjectAltNames. Only used
       *                  when the tlscontext has pvVerifyFQDN set (and possibly when pvCustom is set).
       */
      TLSSocket( bool blocking,
                 SocketParams params,
                 TLSContext& tlscontext,
                 const X509Common::SAN& peer_name );

      /**
       * Destructor.
       */
      virtual ~TLSSocket();

      /**
       * Send data
       * @param buf Data to send
       * @param len Size of data in buf
       * @param more If true, do not force a send buffer flush, more data will follow
       * @return The SystemError code.
       */
      virtual SystemError send( const void* buf, ssize_t len, bool more = false );

      /**
       * Receive data
       * @param buf Buffer to receive in
       * @param request Max bytes to receive in buf
       * @param received Actual received bytes
       * @return The SystemError code.
       */
      virtual SystemError receive( void* buf, ssize_t request, ssize_t &received );

      /**
       * Accept a connection.
       * @return The SystemError.
       */
      virtual TLSSocket* accept();


      /**
       * Connect to the Address.
       * @param address The address to connect to.
       * @return The SystemError code.
       */
      virtual SystemError connect( const Address &address );

      /**
       * Get the peer's certificate.
       * @return A pointer to the peer certificate (must not be freed).
       */
      X509* getPeerCertificate() const;

      /**
       * Identity
       * @param socket The socket to compare to.
       * @return True if both sockets are equal and have the same value in socket_.
       */
      bool operator==(const TLSSocket& socket ) const { return this->socket_ == socket.socket_; };

      /**
       * Ordering
       * @param socket The socket to compare to.
       * @return True if this Socket has a smaller socket descriptor than socket.
       */
      bool operator<(const TLSSocket& socket ) const { return this->socket_ < socket.socket_; };

      /**
       * Assign from Socket.
       * @param socket The Socket to assign/copy to this Socket.
       * @return this TLSSocket.
       */
      TLSSocket& operator=( const TLSSocket& socket );

      /**
       * return the negotiated TLS (SSL) protocol version. This is only meaningfull after a connect returned
       * SystemError::ecOK.
       * @return the TLS (SSL) protocol version.
       */
      int getTLSProtocolVersion() const {
        if ( ssl_ ) return SSL_version( ssl_ ); else return 0;
      }

      /**
       * Return the negotiated TLS (SSL) protocol version. This is only meaningful after a connect returned
       * SystemError::ecOK.
       * @return the TLS (SSL) protocol version.
       */
      string getTLSProtocolVersionString() const {
        if ( ssl_ ) return SSL_get_version( ssl_ ); else return "?";
      }

      /**
       * Return the negotiated cipher name. This is only meaningful after a connect returned
       * SystemError::ecOK.
       * @return the TLS (SSL) protocol version.
       */
      string getTLSCurrentCipherName() const {
        if ( ssl_ ) return SSL_get_cipher_name( ssl_ ); else return "?";
      }

    protected:

      /**
       * The SSL object.
       */
      SSL* ssl_;

      /**
       * The TLSContext
       */
      TLSContext& tlscontext_;

      /**
       * The peer name connected to, for TLS CN and SubjectAltName matching.
       */
      X509Common::SAN peer_name_;

  };

};

#endif
