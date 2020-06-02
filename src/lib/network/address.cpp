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
 * @file address.cpp
 * Implements the dodo::network::Address class.
 */

#include "network/address.hpp"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>


#include <iostream>
#include <sstream>

#include <iostream>

namespace dodo::network {

  Address::Address( const string &address  ) {
    init();
    *this = address;
  }

  Address::Address( const string &ip, uint16_t port ) {
    init();
    *this = ip;
    setPort( port );
  }

  Address::Address( const sockaddr_storage& address ) {
    addr_.ss_family = AF_UNSPEC;
    memcpy( &addr_, &address, sizeof(addr_) );
  }

  Address::Address( const sockaddr *address, socklen_t len ) {
    addr_.ss_family = AF_UNSPEC;
    memcpy( &addr_, address, len );
  }

  Address::Address( const Address &address ) {
    addr_.ss_family = AF_UNSPEC;
    memcpy( &addr_, &(address.addr_), sizeof(addr_) );
  }

  void Address::init() {
    memset( &addr_, 0, sizeof(addr_) );
    addr_.ss_family = AF_UNSPEC;
  }

  Address& Address::operator=( const string& address ) {
    auto rc = inet_pton( AF_INET, address.c_str(), &asIPv4Address()->sin_addr );
    if ( rc != 1 ) {
      rc = inet_pton( AF_INET6, address.c_str(), &asIPv6Address()->sin6_addr );
      if ( rc != 1 ) {
        memset( &addr_, 0, sizeof(addr_) );
        addr_.ss_family = AF_UNSPEC;
      } else {
        addr_.ss_family = AF_INET6;
      }
    } else {
      addr_.ss_family = AF_INET;
    }
    return *this;
  }

  Address& Address::operator=( const Address& address ) {
    memcpy( &addr_, &(address.addr_), sizeof(addr_) );
    return *this;
  }

  Address& Address::operator=( const sockaddr_storage& address ) {
    memcpy( &addr_, &address, sizeof(addr_) );
    return *this;
  }

  bool Address::operator==( const Address& address ) const {
    if ( addr_.ss_family == address.addr_.ss_family ) {
      if ( addr_.ss_family == AF_INET ) {
        sockaddr_in* local = asIPv4Address();
        sockaddr_in* other = address.asIPv4Address();
        return memcmp( local, other, sizeof(sockaddr_in) ) == 0;
      } else if ( addr_.ss_family == AF_INET6 ) {
        sockaddr_in6* local = asIPv6Address();
        sockaddr_in6* other = address.asIPv6Address();
        return memcmp( local, other, sizeof(sockaddr_in6) ) == 0;
      } else return false;
    } else return false;
  }


  string Address::asString( bool withport )  const {
    char ip_[INET6_ADDRSTRLEN];
    stringstream ss;
    if ( addr_.ss_family == AF_INET ) {
      if ( !inet_ntop( AF_INET, &asIPv4Address()->sin_addr, ip_, sizeof(ip_) ) )
        throw_SystemExceptionObject( "inet_ntop failed", errno, this );
    } else if ( addr_.ss_family == AF_INET6 ) {
      if ( !inet_ntop( AF_INET6, &asIPv6Address()->sin6_addr, ip_, sizeof(ip_) ) )
        throw_SystemExceptionObject( "inet_ntop failed", errno, this );
    } else if ( addr_.ss_family == AF_UNSPEC  ) {
      return "invalid address";
    } else {
      return "unhandled address family";
    }
    ss << ip_;
    if ( withport )
      if ( getPort() ) ss << ":" << getPort();
    return ss.str();
  }

  uint16_t Address::getPort() const {
    if ( addr_.ss_family == AF_INET ) {
      return ntohs( asIPv4Address()->sin_port );
    } else if ( addr_.ss_family == AF_INET6 ) {
      return ntohs( asIPv6Address()->sin6_port );
    }
    return 0;
  }

  void Address::setPort( uint16_t port ) {
    if ( addr_.ss_family == AF_INET ) {
      asIPv4Address()->sin_port = htons( port );
    } else if ( addr_.ss_family == AF_INET6 ) {
      asIPv6Address()->sin6_port = htons( port );
    }
  }

  SystemError Address::getHostAddrInfo( const std::string &hostname, AddrInfo& info ) {
    struct addrinfo *ainfo = 0;
    struct addrinfo ahints;
    memset( &ahints, 0, sizeof(ahints) );
    ahints.ai_family = AF_UNSPEC;
    ahints.ai_flags = AI_CANONNAME | AI_CANONIDN | AI_ADDRCONFIG;
    SystemError rc = getaddrinfo( hostname.c_str(),
                                  0,
                                  &ahints,
                                  &ainfo );
    if ( rc == SystemError::ecOK ) {
      struct addrinfo *rp = 0;
      for ( rp = ainfo; rp != NULL; rp = rp->ai_next ) {
        AddrInfoItem item;
        item.address = Address( rp->ai_addr, rp->ai_addrlen );
        item.params.setAddressFamily( SocketParams::AddressFamily( rp->ai_family ) );
        item.params.setSocketType( SocketParams::SocketType( rp->ai_socktype ) );
        item.params.setProtocol( SocketParams::ProtocolNumber(rp->ai_protocol) );
        if ( rp->ai_canonname ) info.canonicalname = rp->ai_canonname;
        info.items.push_back( item );
      }
      if ( ainfo ) freeaddrinfo( ainfo );
    } else if ( rc == SystemError::ecEAI_ADDRFAMILY ||
                rc == SystemError::ecEAI_NODATA ||
                rc == SystemError::ecEAI_NONAME ) {
      if ( ainfo ) freeaddrinfo( ainfo );
    } else {
      freeaddrinfo( ainfo );
      throw_SystemException( "Address::getHostAddrInfo failed", rc );
    }
    return rc;
  }

  SystemError Address::getHostAddrInfo( const std::string &hostname, SocketParams &params, Address &address, string &canonicalname ) {
    address = Address();
    canonicalname = "";
    AddrInfo info;
    SystemError error = getHostAddrInfo( hostname, info );
    if ( error == SystemError::ecOK ) {
      if ( info.items.size() < 1 ) return SystemError::ecEAI_NODATA;
      for ( auto item : info.items ) {
        if ( item.params.getAddressFamily() == params.getAddressFamily() || params.getAddressFamily() == SocketParams::afUNSPEC ) {
          if ( item.params.getSocketType() == params.getSocketType() ) {
            if ( item.params.getProtocol() == params.getProtocol() ) {
              canonicalname = info.canonicalname;
              address = item.address;
              params.setAddressFamily( address.getAddressFamily() );
              return SystemError::ecOK;
            } else return SystemError::ecEAI_NODATA;
          } else return SystemError::ecEAI_NODATA;
        } else return SystemError::ecEAI_NODATA;
      }
      return SystemError::ecOK;
    } else return error; //SystemError::ecEAI_NODATA;
  }

  SystemError Address::getNameInfo( std::string &hostname ) const {
    char hbuf[NI_MAXHOST];
    auto error = getnameinfo( (const sockaddr*) &addr_,
                              sizeof(addr_),
                              hbuf,
                              sizeof(hbuf),
                              NULL,
                              0,
                              NI_NAMEREQD );
    if ( error == SystemError::ecEAI_SYSTEM ) error = errno;
    if ( !error ) hostname = hbuf; else hostname = "";
    return error;
  }

  SystemError Address::getNameInfo( std::string &hostname, std::string &service ) const {
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    auto error = getnameinfo( (const sockaddr*) &addr_,
                              sizeof(addr_),
                              hbuf,
                              sizeof(hbuf),
                              sbuf,
                              sizeof(sbuf),
                              NI_NAMEREQD );
    if ( error == SystemError::ecEAI_SYSTEM ) error = errno;
    if ( !error ) hostname = hbuf; else hostname = "";
    return error;
  }

  SystemError Address::getNameInfo( std::string &hostname, uint16_t &port ) const {
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    auto error = getnameinfo( (const sockaddr*) &addr_,
                              sizeof(addr_),
                              hbuf,
                              sizeof(hbuf),
                              sbuf,
                              sizeof(sbuf),
                              NI_NAMEREQD | NI_NUMERICSERV );
    if ( error == SystemError::ecEAI_SYSTEM ) error = errno;
    if ( !error ) {
      hostname = hbuf;
      errno = 0;
      int p = atoi( sbuf );
      if ( errno ) throw_SystemException( "atoi faiure reading numeric port", errno );
      port = static_cast<uint16_t>( p );
    } else {
      hostname = "";
      port = 0;
    }
    return error;
  }

}
