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
 * @file sslsocket.hpp
 * Defines the arca::network::SSLSocket class.
 */

#ifndef network_sslsocket_hpp
#define network_sslsocket_hpp

#include "network/socketparams.hpp"
#include "network/sslcontext.hpp"
#include "network/socket.hpp"


namespace dodo::network {

  //using namespace std;

  /**
   * Socket for SSL encrypted traffic between trusted endpoints.
   */
  class SSLSocket : public BaseSocket {
    public:

      /**
       * Construct from existing socket file descriptor.
       */
      SSLSocket( int socket, SSLContext& sslcontext );

      /**
       * Construct from scratch.
       */
      SSLSocket( bool blocking,
                 SocketParams params,
                 SSLContext& sslcontext );

      /**
       * Destructor.
       */
      virtual ~SSLSocket();

      /**
       * Connect to the Address.
       */
      virtual SystemError connect( const Address &address );

      /**
       * Send data
       * @param buf Data to send
       * @param len Sizeof data in buf
       * @param more If true, do not force a send buffer flush, more data will follow
       */
      virtual SystemError send( const void* buf, ssize_t len, bool more = false );

      /**
       * Receive data
       * @param buf Buffer to receive in
       * @param request Max bytes to receive in buf
       * @param received Actual received bytes
       */
      virtual SystemError receive( void* buf, ssize_t request, ssize_t &received );

      /**
       * Accept a connection.
       */
      SystemError accept();

      /**
       * Identity
       * @param socket The socket to compare to.
       * @return True if both sockets are equal and have the sxame value of socket_.
       */
      bool operator==(const SSLSocket& socket ) const { return this->socket_ == socket.socket_; };

      /**
       * Ordering
       * @param socket The socket to compare to.
       * @return True if this Socket has a smaller socket descriptor than socket.
       */
      bool operator<(const SSLSocket& socket ) const { return this->socket_ < socket.socket_; };

      /**
       * Assign from Socket.
       * @param socket The Socket to assign/copy to this Socket.
       */
      SSLSocket& operator=( const SSLSocket& socket );

    private:
      /**
       * The SSL object.
       */
      SSL* ssl_;

      /**
       * The SSLContext
       */
      SSLContext& sslcontext_;

  };

};

#endif
