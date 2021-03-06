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
 * @file socket.hpp
 * Defines the dodo::network::Socket class.
 */

#ifndef network_socket_hpp
#define network_socket_hpp

#include <arpa/inet.h>

#include <map>
#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>

#include "common/systemerror.hpp"
#include "network/address.hpp"
#include "network/basesocket.hpp"

namespace dodo::network {

  /**
   * A Linux socket.
   * Socket objects refer to, but do not own a particular socket descriptor. The programmer controls closing the socket
   * by calling close(), the Socket destructor does not do this implicitly, as two Socket objects may refer to the
   * same socket descriptor.
   *
   * The Socket operates either in blocking or non-blocking mode. In blocking mode, calls to listen() or receive()
   * will block until data is received. Moreover, in blocking mode, there is no limit to the amount of data that can be
   * sent or received in one go. In non-blocking mode, the send and receive sizes per call must fit in
   * Socket::getSendBufSize() or Socket::getReceiveBufSize().
   */
  class Socket : public BaseSocket {
    public:


      /**
       * Default constructor creates an invalid socket.
       */
      Socket() : BaseSocket() {};

      /**
       * Construct from a socket descriptor.
       * @param socket The socket file descriptor.
       */
      Socket( int socket ) : BaseSocket(socket) {};

      /**
       * Construct from SocketParams.
       * @param blocking If true, create the socket in blocking mode.
       * @param params The SocketParams to use.
       */
      Socket( bool blocking, SocketParams params ) : BaseSocket( blocking, params ) {};

      /**
       * Construct with default SocketParams.
       * @param blocking If true, create the socket in blocking mode.
       */
      Socket( bool blocking ) : BaseSocket( blocking ) {};

      /**
       * Destructs this Socket, but does not call close().
       */
      virtual ~Socket() {};

      /**
       * Send len bytes as a bytestream. connect must have been called first.
       * @param buf The bytes to send.
       * @param len The number of bytes to send from buf.
       * @param more If true, do no forec a network send right away, but wait until send buffer full or
       * a subsequent call with more set to false.
       * @return SystemError::ecOK or
       *   - SystemError::ecEAGAIN: For non-blocking sockets, neither failure nor succces, try again.
       *   - SystemError::ecECONNRESET : The peer has disconnected during the send.
       * @see sendTo()
       */
      virtual common::SystemError send( const void* buf, ssize_t len, bool more = false );

      /**
       * Send raw packets to the given Address.
       * @param address The destination Address
       * @param buf The bytes to send
       * @param len The number of bytes to send.
       * @return The SystemError.
       */
      virtual common::SystemError sendTo( const Address& address, const void* buf, ssize_t len );

      /**
       * Receive bytes
       * @param buf The buffer to receive the data into.
       * @param request The number of bytes to receive (request <= sizeof(buf) ).
       * @param received The number of bytes actuially received
       * @return The error code:
       *   -  SystemError::ecOK
       *   -  SystemError::ecEAGAIN
       *   -  SystemError::ecENOTCONN
       *   -  SystemError::ecECONNREFUSED
       * On other errors returned by the internal call to recv, an exception is thrown.
       */
      virtual common::SystemError receive( void* buf, ssize_t request, ssize_t &received );

      /**
       * Accepts a connection request and return a pointer to a new Socket for the new connection, the caller will
       * own the new Socket object.
       * @return The accepted socket.
       */
      virtual Socket* accept();

      /**
       * An invalid SocketInvalid for comparison convenience - initialized to a Socket with socket fd=-1.
       */
      static Socket SocketInvalid;

    protected:


  };


}

#endif
