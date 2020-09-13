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
 * @file basesocket.hpp
 * Defines the dodo::network::BaseSocket class.
 */

#ifndef network_basesocket_hpp
#define network_basesocket_hpp

#include <fcntl.h>

#include "common/exception.hpp"
#include "network/address.hpp"

namespace dodo::network {

  /**
   * Interface to and common implementation of concrete sockets (Socket, TLSSocket).
   */
  class BaseSocket : public common::DebugObject {
    public:

      /**
       * Default constructor creates an invalid socket.
       */
      BaseSocket();

      /**
       * Construct from a socket descriptor.
       * @param socket The socket file descriptor.
       */
      BaseSocket( int socket );

      /**
       * Construct from SocketParams.
       * @param blocking If true, create the socket in blocking mode.
       * @param params The SocketParams to use.
       */
      explicit BaseSocket( bool blocking, SocketParams params );

      /**
       * Destructs this Socket, but does not call close().
       */
      virtual ~BaseSocket() {};

      /**
       * Return debug object state as a string.
       * @return The string.
       */
      virtual std::string debugDetail() const;

      /**
       * Connect to the address. These common::SystemError may be returned:
       *
       *   - SystemError::ecOK
       *   - SystemError::ecEACCES
       *   - SystemError::ecEADDRINUSE
       *   - SystemError::ecEADDRNOTAVAIL
       *   - SystemError::ecEALREADY
       *   - SystemError::ecECONNREFUSED
       *   - SystemError::ecEINPROGRESS
       *   - SystemError::ecEISCONN
       *   - SystemError::ecENETUNREACH
       *   - SystemError::ecETIMEDOUT
       *
       * In case the system returns other system errors on connect, a common::Exception is thrown.
       * @param address The address to connect to.
       * @return The SystemError
       */
      virtual SystemError connect( const Address &address );

      /**
       * Closes the socket, causing the connection, if it exists, to be terminated.
       * @return nothing
       */
      virtual void close();

      /**
       * Send bytes on the socket.
       * @param buf Take bytes from here
       * @param len The number of bytes.
       * @param more Set to true if more is to come (increases packet filling efficiency when accurate).
       * @return The SystemError.
       */
      virtual SystemError send( const void* buf, ssize_t len, bool more = false ) = 0;

      /**
       * Receive bytes from the socket.
       * @param buf Put bytes received here
       * @param request The maximum number of bytes to receive
       * @param received The number of bytes received
       * @return The SystemError.
       */
      virtual SystemError receive( void* buf, ssize_t request, ssize_t &received ) = 0;

      /**
       * Rerurn true if the socket is operating in blocking mode.
       * @return The mode.
       */
      virtual bool getBlocking() const;

      /**
       * Return the socket file descriptor.
       * @return The socket descriptor.
       */
      int getFD() const { return socket_; };

      /**
       * Return true when the socket descriptor is a valid, hence 'possible' descriptor.
       * A valid decsriptor does not have to be open/connected or even correct - a valid decsriptor is not -1.
       * @return true of the socket is a valid descriptor ( >= 0 );
       */
      bool isValid() const { return socket_ >= 0; };

      /**
       * Set the Socket blocking mode.
       * @param blocking Blocking if true.
       */
      void setBlocking( bool blocking );

      /**
       * Set TCP_NODELAY. If true, disable Nagle's algorithm.
       * @param set If true, set TCP_NODELAY.
       */
      void setTCPNoDelay( bool set );

      /**
       * Enable the socket to re-use an address when listen/bind is called.
       * In transit TCP messages sent to a server previously listening on this address
       * will not be picked up by the re-used address.
       * @see setReUsePort()
       */
      void setReUseAddress();

      /**
       * Make the socket re-use a port when listen is called.
       * @see setReUseAddress()
       */
      void setReUsePort();

      /**
       * Get the maximum buffer length for send.
       * @return The send buffer size.
       * @see setSendBufSize
       */
      socklen_t getSendBufSize() const;

      /**
       * Get the maximum buffer length for receive.
       * @return The receive buffer size.
       * @see setReceiveBufSize()
       */
      virtual socklen_t getReceiveBufSize() const;

      /**
       * Get the Socket TTL (time to live) or the max number of packet hops.
       * @return The current socket TTL value.
       */
      int getTTL() const;

      /**
       * Set the size of the send buffer. Linux will (at least) double the memory for socket accounting purposes.
       * If size < system minimum size or size > system maximum size, the system will silently overrule.
       * Also note that getSendBufSize() and getReceiveBufSize() return the size of the kernel send buffer divided by
       * two, as the Linux man 7 socket states:
       * "...Linux assumes that half of the send/receive buffer is used for internal kernel structures;"
       * So getSendBufSize() and getReceiveBufSize() return the values from a programmer perspective - it is the number
       * of bytes the programmer can actually send or receive in one go
       * @code
       * Socket s;
       * s.setSendBufSize(8192);          // kernel has set it to 16384
       * int n = s.getSendBufSize();      // n = 8192
       * char buffer[s.getSendBufSize()]; // which is what you asked for and will fit
       * @endcode
       * @param size The size to set the send buffer to.
       */
      void setSendBufSize( socklen_t size );

      /**
       * Set the Socket TTL (Time to Live).
       * @param ttl The ttl value to set.
       * @see getTTL()
       */
      void setTTL( int ttl );

      /**
       * Set the size of the receive buffer. Linux will (at least) double the memory for socket accounting purposes.
       * If size < system minimum size, the system will set the latter.
       * @see setSendBufSize( socklen_t size )
       * @param size The size to set the receive buffer to.
       */
      void setReceiveBufSize( socklen_t size );

      /**
       * Set the receive timeout - a receive will fail if there is no data received before the timeout expires.
       * Note that the default is 0 seconds, meaning no timeout / wait forever.
       * @param sec Timeout value, in seconds, fractions allowed.
       */
      void setReceiveTimeout( double sec );

      /**
       * Set the send timeout - a send will fail if there is no data send before the timeout expires.
       * Note that the default is 0 seconds, meaning no timeout / wait forever.
       * @param sec Timeout value, in seconds, fractions allowed.
       */
      void setSendTimeout( double sec );

      /**
       * Enable or disable TCP keep-alive on the socket. This might be useful to keep connections ope that run
       * through a firewall that disconnects connections that are idle. Note that keep-alive itself is configured
       * through the Linux kernel. Note that enabling keep-alive will not change the behavior of the socket
       * from a developers perspective but ti does incur a bit of network overhead.
       * @param enable If true enable, otherwise disable.
       */
      void setTCPKeepAlive( bool enable );

      /**
       * Get the local address for this socket.
       * @return The local Address.
       */
      Address getAddress() const;

      /**
       * Get the SocketParams::AddressFamily of the socket.
       * @return The address family.
       */
      SocketParams::AddressFamily getAddressFamily() const;

      /**
       * Get the SocketParams::SocketType of the socket.
       * @return The SocketType.
       */
      SocketParams::SocketType getSocketType() const;

      /**
       * Get the SocketParams::ProtocolNumber of the socket.
       * @return The ProtocolNumber.
       */
      SocketParams::ProtocolNumber getProtocolNumber() const;

      /**
       * Return the SocketParams.
       * @see getAddressFamily(), getSocketType(), getProtocolNumber()
       * @return the SocketParams.
       */
      SocketParams getSocketParams() const;

      /**
       * Get the peer (remote) address for this socket.
       * @return The peer Address.
       */
      Address getPeerAddress() const;

      /**
       * Add identity
       * @param socket The socket to compare to.
       * @return True if both sockets are equal and have the sxame value of socket_.
       */
      bool operator==(const BaseSocket& socket ) const { return this->socket_ == socket.socket_; };

      /**
       * Add ordering
       * @param socket The socket to compare to.
       * @return True if this Socket has a smaller socket descriptor than socket.
       */
      bool operator<(const BaseSocket& socket ) const { return this->socket_ < socket.socket_; };

      /**
       * Assign from existing socket descriptor (int).
       * @param socket The socket value to assign to this Socket.
       * @return This Socket.
       */
      BaseSocket& operator=( int socket );

      /**
       * Assign from Socket.
       * @param socket The Socket to assign/copy to this Socket.
       * @return This Socket.
       */
      BaseSocket& operator=( const BaseSocket& socket );

      /**
       * Sets up a listening socket on Address.
       * @param address The Address (including a port) to listen on.
       * @param backlog The size of the queue used to cache pending connections.
       * @return A common::SystemError, if not ecOK
       *   - common::SystemError::ecEADDRINUSE
       *   - common::SystemError::ecEBADF
       *   - common::SystemError::ecENOTSOCK
       *   - common::SystemError::ecEOPNOTSUPP
       *
       * or one of the common::SystemError returned by bind( const Address &address ).
       */
      SystemError listen( const Address &address, int backlog );

      /**
       * Bind the socket to the Address.
       * @param address The address to bind to.
       * @return SystemError::ecOK or
       *   - SystemError::ecEACCES
       *   - SystemError::ecEADDRINUSE
       *   - SystemError::ecEBADF
       *   - SystemError::ecEINVAL
       *   - SystemError::ecENOTSOCK
       */
      SystemError bind( const Address &address );

      /**
       * Accepts a connection request and return a pointer to a new Socket for the new connection, the caller will
       * own the new Socket object.
       * @return The accepted socket.
       */
      virtual BaseSocket* accept() = 0;

      /**
       * Send an uint8_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveUInt8( uint8_t& )
       */
      SystemError sendUInt8( uint8_t value, bool more = false );

      /**
       * receive an uint8_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendUInt8( uint8_t )
       */
      SystemError receiveUInt8( uint8_t &value );

      /**
       * Send an uint16_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveUInt16( uint16_t& )
       */
      SystemError sendUInt16( uint16_t value, bool more = false );

      /**
       * receive an uint16_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendUInt16( uint16_t )
       */
      SystemError receiveUInt16( uint16_t &value );

      /**
       * Send an uint32_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveUInt32( uint32_t& )
       */
      SystemError sendUInt32( uint32_t value, bool more = false );

      /**
       * receive an uint32_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendUInt32( uint32_t )
       */
      SystemError receiveUInt32( uint32_t &value );

      /**
       * Send an sendUInt64.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveUInt64( uint64_t& )
       */
      SystemError sendUInt64( uint64_t value, bool more = false );

      /**
       * receive an uint64_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendUInt64( uint64_t )*
       */
      SystemError receiveUInt64( uint64_t &value );

      /**
       * Send an int8_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveInt8( int8_t& )
       */
      SystemError sendInt8( int8_t value, bool more = false );

      /**
       * receive an int8_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendInt8( int8_t )*
       */
      SystemError receiveInt8( int8_t &value );

      /**
       * Send an int16_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveInt16( int16_t& )
       */
      SystemError sendInt16( int16_t value, bool more = false );

      /**
       * receive an int16_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendInt16( int16_t )*
       */
      SystemError receiveInt16( int16_t &value );

      /**
       * Send an int32_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveInt32( int32_t& )
       */
      SystemError sendInt32( int32_t value, bool more = false );

      /**
       * receive an int32_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendInt32( int32_t )*
       */
      SystemError receiveInt32( int32_t &value );

      /**
       * Send an int64_t.
       * @param value The value to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       * @see receiveInt64( int64_t& )
       */
      SystemError sendInt64( int64_t value, bool more = false );

      /**
       * receive an int64_t
       * @param value The value received.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       * @see sendInt64( int64_t )*
       */
      SystemError receiveInt64( int64_t &value );

      /**
       * Sends the std::string as understood by receiveString(). The send prefixes the string length in an uint64_t,
       * the number of bytes (characters) that follow.
       * @param s The string to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       */
      SystemError sendString( const std::string &s, bool more = false );

      /**
       * Receive a std::string as sent by sendString( const std::string& ).
       * @param s The string to receive into.
       * @return Same as receive( void* buf, ssize_t request, ssize_t &received )
       */
      SystemError receiveString( string &s );

      /**
       * Sends the std::string terminated by a '\n'.
       * @param s The string to send.
       * @param more If true, the caller indicates that more data will follow.
       * @return Same as send( const void* buf, ssize_t len )
       */
      SystemError sendLine( const std::string &s, bool more );

      /**
       * receive a stream of characters until a '\n' is read (consumed from the socket read buffer but not
       * added to s).
       * @param s receives the read string.
       * @return The SystemError.
       */
      SystemError receiveLine( string &s );

    protected:

      /**
       * The socket file decsriptor.
       */
      int socket_;

  };

}

#endif
