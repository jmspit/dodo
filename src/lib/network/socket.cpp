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
 * @file socket.cpp
 * Implements the dodo::network::Socket class.
 */

#include "network/socket.hpp"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


#include <iostream>
#include <common/exception.hpp>

namespace dodo::network {

  Socket Socket::SocketInvalid = Socket();

  SystemError Socket::send( const void* buf, ssize_t len, bool more ) {
    int flags = MSG_NOSIGNAL;
    if ( more ) flags = flags | MSG_MORE;
    ssize_t left = len;
    ssize_t rc = 0;
    const char* cp = (const char*)buf;
    while ( left > 0 ) {
      rc = ::send( socket_, cp, left, flags );
      if ( rc < 0 ) break;
      cp += rc;
      left -= rc;
    }
    if ( rc < 0 ) {
      switch ( errno ) {
        case SystemError::ecEAGAIN :
        case SystemError::ecECONNRESET :
          return errno;
        default: throw_SystemExceptionObject( common::Puts() << "Socket::send failed", errno, this );
      };
    } else if ( left ) throw_SystemExceptionObject( common::Puts() << "Socket::send not all bytes send", errno, this );
    else return SystemError::ecOK;
  }

  SystemError Socket::sendTo( const Address& address, const void* buf, ssize_t len ) {
    int flags = 0;
    ssize_t left = len;
    ssize_t rc = 0;
    const char* cp = (const char*)buf;
    while ( left > 0 ) {
      rc = ::sendto( socket_, cp, left, flags, (const sockaddr*)&(address.addr_), sizeof(address.addr_) );
      if ( rc < 0 ) break;
      cp += rc;
      left -= rc;
    }
    if ( rc < 0 ) {
      switch ( errno ) {
        case SystemError::ecEAGAIN :
        case SystemError::ecECONNRESET :
          return errno;
        default: throw_SystemExceptionObject( common::Puts() << "Socket::send failed", errno, this );
      };
    } else if ( left ) throw_SystemExceptionObject( common::Puts() << "Socket::send not all bytes send", errno, this );
    else return SystemError::ecOK;
  }

  SystemError Socket::receive( void* buf, ssize_t request, ssize_t &received ) {
    received = 0;
    ssize_t rc = ::recv( socket_, buf, request, 0 );
    if ( rc < 0 ) {
      switch ( errno ) {
        case SystemError::ecEAGAIN :
        case SystemError::ecECONNREFUSED :
        case SystemError::ecENOTCONN :
          return errno;
        default: throw_SystemExceptionObject( common::Puts() << "Socket::receive failed", errno, this );
      };
    } else {
      received = rc;
      return SystemError::ecOK;
    }
  }

  Socket::Socket( const Socket& socket ) {
    socket_ = socket.getSocket();
  }

  SystemError Socket::bind( const Address &address ) {
    int rc = ::bind( socket_, (const sockaddr*)&(address.addr_), sizeof(address.addr_) );
    if ( rc < 0 ) {
      switch ( errno ) {
        case SystemError::ecEACCES:
        case SystemError::ecEADDRINUSE:
          return errno;
      }
      throw_SystemExceptionObject( common::Puts() << "Socket::bind failed on address " << address.asString(), errno, this );
    } else return SystemError::ecOK;
  }

  SystemError Socket::listen( const Address &address, int backlog ) {
    SystemError error = bind( address );
    if ( error == SystemError::ecOK ) {
      int rc = ::listen( socket_, backlog );
      if ( rc < 0 ) {
        switch ( errno ) {
          case SystemError::ecEADDRINUSE:
            return errno;
        }
        throw_SystemExceptionObject( common::Puts() << "Socket::listen failed on " << address.asString(), errno, this );
      } else return SystemError::ecOK;
    } else return error;
  }

  Socket* Socket::accept() {
    Socket* ret = new Socket();
    *ret = Socket::SocketInvalid;
    int rc = ::accept( socket_, nullptr, nullptr );
    if ( rc == -1 ) {
      if ( errno != EAGAIN && errno != EWOULDBLOCK )
        throw_SystemExceptionObject( common::Puts() << "Socket::accept failed", errno, this );
      else
        *ret = rc;
    } else *ret = rc;
    return ret;
  }

}


