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
 * @file basesocket.cpp
 * Implements the dodo::network::BaseSocket class.
 */

#include <common/logger.hpp>
#include "network/basesocket.hpp"
#include <netinet/tcp.h>
#include <unistd.h>
#include <cmath>

namespace dodo::network {

  BaseSocket::BaseSocket() {
    socket_ = -1;
  }

  BaseSocket::BaseSocket( int socket ) {
    socket_ = socket;
  }

  BaseSocket::BaseSocket( bool blocking, SocketParams params ) {
    socket_ = socket( params.getAddressFamily(), params.getSocketType(), params.getProtocol() );
    if ( socket_ == -1 ) throw_SystemExceptionObject( "socket allocation failed", errno, this );
    setBlocking( blocking );
    if ( params.getSocketType() == SocketParams::stSTREAM ) setTCPNoDelay( true );
  }

  std::string BaseSocket::debugDetail() const {
    stringstream ss;
    ss << "socketfd=" << socket_;
    return ss.str();
  }

  SystemError BaseSocket::connect( const Address &address ) {
    auto rc = ::connect( socket_, (const sockaddr*)address.getAddress(), sizeof(sockaddr_storage) );
    if ( rc < 0 ) {
      switch ( errno ) {
        case SystemError::ecEACCES :
        case SystemError::ecEADDRINUSE :
        case SystemError::ecEADDRNOTAVAIL :
        case SystemError::ecEALREADY :
        case SystemError::ecECONNREFUSED :
        case SystemError::ecEINPROGRESS :
        case SystemError::ecEISCONN :
        case SystemError::ecENETUNREACH :
        case SystemError::ecETIMEDOUT :
          return errno;
        default: throw_SystemExceptionObject( "socket connect failed", errno, this );
      };
    } else return SystemError::ecOK;
  }

  void BaseSocket::close() {
    if ( socket_ != -1 ) {
      auto rc = ::close( socket_ );
      socket_ = -1;
      if ( rc == -1 ) throw_SystemExceptionObject( "socket close failed", errno, this );
    }
  }

  BaseSocket& BaseSocket::operator=( int socket ) {
    if ( socket_ != socket ) {
      socket_ = socket;
    }
    return *this;
  }

  BaseSocket& BaseSocket::operator=( const BaseSocket& socket ) {
    if ( socket_ != socket.socket_ ) {
      socket_ = socket.socket_;
    }
    return *this;
  }

  SystemError BaseSocket::listen( const Address &address, int backlog ) {
    SystemError error = bind( address );
    if ( error == SystemError::ecOK ) {
      int rc = ::listen( socket_, backlog );
      if ( rc < 0 ) {
        switch ( errno ) {
          case SystemError::ecEADDRINUSE:
            return errno;
        }
        throw_SystemExceptionObject( "Socket::listen failed on " << address.asString(), errno, this );
      } else return SystemError::ecOK;
    } else return error;
  }

  SystemError BaseSocket::bind( const Address &address ) {
    int rc = ::bind( socket_, (const sockaddr*)&(address.addr_), sizeof(address.addr_) );
    if ( rc < 0 ) {
      switch ( errno ) {
        case SystemError::ecEACCES:
        case SystemError::ecEADDRINUSE:
          return errno;
      }
      throw_SystemExceptionObject( "Socket::bind failed on address " << address.asString(), errno, this );
    } else return SystemError::ecOK;
  }

  void BaseSocket::setTCPNoDelay( bool set ) {
    int value = set?1:0;
    setsockopt( socket_, SOL_TCP, TCP_NODELAY, &value, sizeof(value) );
  }

  void BaseSocket::setTCPKeepAlive( bool enable ) {
    int value = enable?1:0;
    setsockopt( socket_, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value) );
  }

  bool BaseSocket::getBlocking() const {
    int flags = fcntl( socket_, F_GETFL, 0 );
    if ( flags == -1 ) throw_SystemExceptionObject( "fcntl failed", errno, this );
    return ! (flags & O_NONBLOCK);
  }

  void BaseSocket::setBlocking( bool blocking ) {
    if ( !isValid() ) throw_Exception( "setBlocking on invalid socket" );
    int flags = fcntl( socket_, F_GETFL, 0 );
    if ( flags == -1 ) throw_SystemExceptionObject( "fcntl failed", errno, this );
    if ( blocking ) {
      if ( flags & O_NONBLOCK ) {
        int rc = fcntl( socket_, F_SETFL, flags ^ O_NONBLOCK );
        if ( rc < 0 ) throw_SystemExceptionObject( "setsockopt failed", errno, this );
      }
    } else {
      if ( !(flags & O_NONBLOCK) ) {
        int rc = fcntl( socket_, F_SETFL, flags | O_NONBLOCK );
        if ( rc < 0 ) throw_SystemExceptionObject( "setsockopt failed", errno, this );
      }
    }
  }

  void BaseSocket::setReUseAddress() {
    if ( !isValid() ) throw_Exception( "setReUseAddress on invalid socket" );
    int opt_enable = 1;
    auto rc = setsockopt( socket_,
                          SOL_SOCKET,
                          SO_REUSEADDR,
                          (char *)&opt_enable,
                          sizeof(opt_enable) );
    if ( rc < 0 ) throw_SystemExceptionObject( "setsockopt SO_REUSEADDR failed", rc, this );
  }

  void BaseSocket::setReUsePort() {
    int opt_enable = 1;
    auto rc = setsockopt( socket_,
                          SOL_SOCKET,
                          SO_REUSEPORT,
                          (char *)&opt_enable,
                          sizeof(opt_enable) );
    if ( rc < 0 ) throw_SystemExceptionObject( "setsockopt SO_REUSEPORT failed", rc, this );
  }

  socklen_t BaseSocket::getSendBufSize() const {
    socklen_t optvar  = 0;
    socklen_t optlen = sizeof(optvar);
    auto rc = getsockopt( socket_, SOL_SOCKET, SO_SNDBUF, &optvar, &optlen );
    if ( rc == -1 ) throw_SystemExceptionObject( "getSendBufSize failed", errno, this );
    return optvar;
  }

  socklen_t BaseSocket::getReceiveBufSize() const {
    socklen_t optvar  = 0;
    socklen_t optlen = sizeof(optvar);
    auto rc = getsockopt( socket_, SOL_SOCKET, SO_RCVBUF, &optvar, &optlen );
    if ( rc == -1 ) throw_SystemExceptionObject( "getReceiveBufSize failed", errno, this );
    return optvar;
  }

  int BaseSocket::getTTL() const {
    int optvar  = 0;
    socklen_t optlen = sizeof(optvar);
    int rc = -1;
    if ( getAddressFamily() == SocketParams::afINET ) {
      rc = getsockopt( socket_, SOL_IP, IP_TTL, &optvar, &optlen );
    }
    else if ( getAddressFamily() == SocketParams::afINET6 ) {
      rc = getsockopt( socket_, SOL_IP, IP_TTL, &optvar, &optlen );
    }
    if ( rc == -1 ) throw_SystemExceptionObject( "getTTL failed", errno, this );
    return optvar;
  }

  void BaseSocket::setSendBufSize( socklen_t size ) {
    auto rc = setsockopt( socket_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size) );
    if ( rc == -1 ) throw_SystemExceptionObject( "setSendBufSize failed", errno, this );
  }

  void BaseSocket::setReceiveBufSize( socklen_t size ) {
    int rc = setsockopt( socket_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size) );
    if ( rc == -1 ) throw_SystemExceptionObject( "setReceiveBufSize failed", errno, this );
  }

  void BaseSocket::setReceiveTimeout( double sec ) {
    double frac, whole;
    frac = modf ( sec , &whole );
    struct timeval tv_out;
    tv_out.tv_sec = (time_t)whole;
    tv_out.tv_usec = (time_t)(frac * 1.0E6);
    int rc = setsockopt( socket_, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out) );
    if ( rc == -1 ) throw_SystemExceptionObject( "setReceiveTimeout failed", errno, this );
  }

  void BaseSocket::setSendTimeout( double sec ) {
    double frac, whole;
    frac = modf ( sec , &whole );
    struct timeval tv_out;
    tv_out.tv_sec = (time_t)whole;
    tv_out.tv_usec = (time_t)(frac * 1.0E6);
    int rc = setsockopt( socket_, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out) );
    if ( rc == -1 ) throw_SystemExceptionObject( "setSendTimeout failed", errno, this );
  }

  void BaseSocket::setTTL( int ttl ) {
    int rc = -1;
    rc = setsockopt( socket_, SOL_IP, IP_TTL, &ttl, sizeof(ttl) );
    if ( errno == 92 ) rc = setsockopt( socket_, SOL_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl) );
    if ( rc == -1 ) throw_SystemExceptionObject( "setTTL failed", errno, this );
  }

  Address BaseSocket::getAddress() const {
    sockaddr_storage addr;
    socklen_t sz = sizeof(addr);
    int rc = getsockname( socket_,  (sockaddr*)&addr, &sz );
    if ( rc < 0 ) addr.ss_family = AF_UNSPEC;
    return Address(addr);
  }

  SocketParams::AddressFamily BaseSocket::getAddressFamily() const {
    socklen_t optvar  = 0;
    socklen_t optlen = sizeof(optvar);
    int rc = getsockopt( socket_, SOL_SOCKET, SO_DOMAIN, &optvar, &optlen );
    if ( rc == -1 ) throw_SystemExceptionObject( "getAddressFamily failed", errno, this );
    return (SocketParams::AddressFamily)optvar;
  }

  SocketParams::SocketType BaseSocket::getSocketType() const {
    socklen_t optvar  = 0;
    socklen_t optlen = sizeof(optvar);
    int rc = getsockopt( socket_, SOL_SOCKET, SO_TYPE, &optvar, &optlen );
    if ( rc == -1 ) throw_SystemExceptionObject( "getSocketType failed", errno, this );
    return (SocketParams::SocketType)optvar;
  }

  SocketParams::ProtocolNumber BaseSocket::getProtocolNumber() const {
    socklen_t optvar  = 0;
    socklen_t optlen = sizeof(optvar);
    int rc = getsockopt( socket_, SOL_SOCKET, SO_PROTOCOL, &optvar, &optlen );
    if ( rc == -1 ) throw_SystemExceptionObject( "getProtocolNumber failed", errno, this );
    return (SocketParams::ProtocolNumber)optvar;
  }

  SocketParams BaseSocket::getSocketParams() const {
    SocketParams p( getAddressFamily(), getSocketType(), getProtocolNumber() );
    return p;
  }

  Address BaseSocket::getPeerAddress() const {
    sockaddr_storage addr;
    socklen_t sz = sizeof(addr);
    int rc = getpeername( socket_,  (sockaddr*)&addr, &sz );
    if ( rc < 0 ) addr.ss_family = AF_UNSPEC;
    return Address(addr);
  }

  SystemError BaseSocket::sendUInt8( uint8_t value, bool more ) {
    uint8_t nwbo = value;
    return send( &nwbo, sizeof(nwbo) );
  }

  SystemError BaseSocket::receiveUInt8( uint8_t& value ) {
    uint8_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = t;
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendUInt16( uint16_t value, bool more ) {
    uint16_t nwbo = htons( value );
    return send( &nwbo, sizeof(nwbo) );
  }

  SystemError BaseSocket::receiveUInt16( uint16_t& value ) {
    uint16_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error ==  SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = ntohs(t);
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendInt8( int8_t value, bool more ) {
    int8_t nwbo = value;
    return send( &nwbo, sizeof(nwbo), more );
  }

  SystemError BaseSocket::receiveInt8( int8_t& value ) {
    int8_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = t;
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendInt16( int16_t value, bool more ) {
    int16_t nwbo = htons( value );
    return send( &nwbo, sizeof(nwbo), more );
  }

  SystemError BaseSocket::receiveInt16( int16_t& value ) {
    int16_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = ntohs(t);
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendUInt32( uint32_t value, bool more )  {
    uint32_t nwbo = htonl( value );
    return send( &nwbo, sizeof(nwbo), more );
  }

  SystemError BaseSocket::receiveUInt32( uint32_t &value ) {
    uint32_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = ntohl(t);
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendInt32( int32_t value, bool more )  {
    int32_t nwbo = htonl( value );
    return send( &nwbo, sizeof(nwbo), more );
  }

  SystemError BaseSocket::receiveInt32( int32_t &value ) {
    int32_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = ntohl(t);
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendUInt64( uint64_t value, bool more ) {
    uint64_t nwbo = htobe64( value );
    return send( &nwbo, sizeof(nwbo), more );
  }

  SystemError BaseSocket::receiveUInt64( uint64_t &value ) {
    uint64_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = be64toh( t );
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendInt64( int64_t value, bool more ) {
    int64_t nwbo = htobe64( value );
    return send( &nwbo, sizeof(nwbo), more );
  }

  SystemError BaseSocket::receiveInt64( int64_t &value ) {
    int64_t t = 0;
    ssize_t request = sizeof(t);
    ssize_t received = 0;
    ssize_t total = 0;
    SystemError error;
    do {
      error = receive( &t, request, received );
      total += received;
    } while ( error == SystemError::ecOK && total < request );
    if ( error == SystemError::ecOK ) {
      value = be64toh( t );
    } else value = 0;
    return error;
  }

  SystemError BaseSocket::sendString( const std::string &s, bool more ) {
    if ( getBlocking() ) {
      SystemError error;
      error = sendUInt64( s.size(), true );
      if ( error != SystemError::ecOK ) return error;
      error = send( s.c_str(), s.size(), more );
      return error;
    } else throw_Exception( "sendString used on a non-blocking socket" );
  }

  SystemError BaseSocket::receiveString( string &s ) {
    s = "";
    SystemError error;
    if ( getBlocking() ) {
      char buf[getReceiveBufSize()+1];
      uint64_t stringsize = 0;
      error = receiveUInt64( stringsize );
      if (  error == SystemError::ecOK ) {
        ssize_t received = 0;
        uint64_t total = 0;
        do {
          error = receive( buf, stringsize, received );
          total += received;
          buf[received] = 0;
          s += buf;
        } while ( error == SystemError::ecOK && total < stringsize );
        return error;
      } else return error;
    } else throw_ExceptionObject( "receiveString used on a non-blocking socket", this );
  }

  SystemError BaseSocket::sendLine( const std::string &s, bool more ) {
    SystemError error;
    std::stringstream ss;
    ss << s << '\n';
    error = send( ss.str().c_str(), ss.str().length(), more );
    return error;
  }

  SystemError BaseSocket::receiveLine( string &s ) {
    SystemError error = SystemError::ecOK;
    char c = 0;
    ssize_t received = 0;
    std::stringstream ss;
    error = receive( &c, 1, received );
    while ( error == SystemError::ecOK && received == 1 && c != '\n' ) {
      if ( c != '\r' ) ss << c;
      error = receive( &c, 1, received );
    }
    if ( error == SystemError::ecOK ) s = ss.str(); else s = "";
    return error;
  }

}
