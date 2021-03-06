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
 * @file address.hpp
 * Defines the dodo::network::Address class.
 */

#ifndef network_address_hpp
#define network_address_hpp

#include <arpa/inet.h>
#include <iostream>
#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include <sys/socket.h>

#include "common/exception.hpp"
#include "network/socketparams.hpp"

namespace dodo::network {

  struct AddrInfo;

  /**
   * Generic network Address, supporting ipv4 and ipv6 transparently.
   *
   * Typical client use is to first resolve a host name. We specify SocketParams::afUNSPEC
   * for the address family, allowing for both ipv4 and ipv6 and specify the TCP protocol (stSTRAM + pnTCP ).
   *
   * @code
   *     using namespace dodo::common;
   *     using namespace dodo::network;
   *
   *     Address address;
   *     SocketParams sock_params = SocketParams( SocketParams::afUNSPEC,
   *                                              SocketParams::stSTREAM,
   *                                              SocketParams::pnTCP );
   *     std::string canonicalname;
   *     SystemError error = Address::getHostAddrInfo( "httpbin.org", sock_params, address, canonicalname );
   *
   * @endcode
   *
   * The canonical name is set and may differ from the host name passed to getHostAddrInfo.
   *
   * After the getHostAddrInfo (if error == SystemError::ecOK ) the sock_params will have its address family set to
   * either network::SocketParams::afINET or network::SocketParams::afINET6, depending on what the server
   * (httpbin.org) supports. But we can continue to connect without any reference to the actual address family being
   * used:
   *
   * @code
   *     if ( error == SystemError::ecOK ) {
   *       address.setPort( 443 );
   *       Socket socket( true, sock_params );
   *       error = socket.connect( address );
   *       if ( error == SystemError::ecOK ) {
   *         // connected
   *       }
   *     }
   * @endcode
   *
   * There are two prototypes for getHostAddrInfo, one returns the preferred Address (as above), the other a list of
   * addresses (an AddrInfo structure), the preferred address the first in the list. Subsequent calls do not necessarily
   * (under DNS round robin for example) return the same address as preferred address.
   *
   * As the getHostAddrInfo calls are quite expensive, prefer reuse of obtained Address objects where possible.
   *
   * | Special addresses | ipv4 | ipv6 |
   * |---------|------|------|
   * | INADDR_ANY | 0.0.0.0 | ::0 |
   * | localhost | 127.0.0.1 | ::1 |
   *
   */
  class Address : public common::DebugObject {
    public:

      /**
       * Construct an invalid (unspecified) address.
       */
      Address() { addr_.ss_family = AF_UNSPEC; };

      /**
       * Construct from "ip", port
       * @param ip The string representation of the ip, either ipv4 or ipv6.
       * @param port The port number (only specified for listen addresses).
       */
      Address( const std::string &ip, uint16_t port );

      /**
       * Construct from ip, do not set port (only a listening socket needs a port preset).
       * @param ip The string representation of the ip, either ipv4 or ipv6.
       */
      Address( const std::string &ip );

      /**
       * Construct from sockaddr_storage.
       * @param address A sockaddr_storage structure.
       */
      Address( const sockaddr_storage& address );

      /**
       * Construct from sockaddr *.
       * @param address A const sockaddr*
       * @param len And its length.
       */
      Address( const sockaddr *address, socklen_t len );

      /**
       * Copy from Address
       * @param address Source address to copy from.
       */
      Address( const Address &address );

      /**
       * True if this Address is valid.
       * @return true when this Address is valid.
       */
      bool isValid() const { return addr_.ss_family != AF_UNSPEC; };

      /**
       * Return a string representation of this Address.
       * @param withport If true, append ':' + port number.
       * @return string representation of this Address.
       */
      std::string asString( bool withport = false ) const;

      /**
       * Return the port number.
       * @return The port number.
       */
      uint16_t getPort() const;

      /**
       * Set the port number.
       * @param port The new port number.
       */
      void setPort( uint16_t port );

      /**
       * Get this Address family.
       * @return The address family of this Address.
       */
      SocketParams::AddressFamily getAddressFamily() const { return SocketParams::AddressFamily( addr_.ss_family ); };

      /**
       * Get to the underlying sockaddr_storage.
       * @return A pointer to the underlying socket storage.
       */
      const sockaddr_storage* getAddress() const { return &addr_; };

      /**
       * Assign from "ip" string.
       * @param address The address to assign in string form.
       * @return This Address.
       */
      Address& operator=( const std::string& address );

      /**
       * Assign (copy) from Address.
       * @param address The Address to assign.
       * @return This Address.
       */
      Address& operator=( const Address& address );

      /**
       * Assign from sockaddr_storage.
       * @param address The address to assign in sockaddr_storage form.
       * @return This Address.
       */
      Address& operator=( const sockaddr_storage& address );

      /**
       * Test for equality. The comparison is a byte compare on the internal sockaddr_in / sockaddr_in6
       * structures, which guarantees that Address objects assigned "::1" and "0:0:0:0:0:0:0:1" are considered equal.
       * @param address The address to compare to.
       * @return True if the addresses are equal.
       */
      bool operator==( const Address& address ) const;

      /**
       * Get Address information on a host in a AddrInfo struct.
       *
       * Could return the following SystemError
       *   - SystemError::ecOK
       *   - SystemError::ecEAI_ADDRFAMILY (The hosts exists but has not address in the requested address family)
       *   - SystemError::ecEAI_NODATA (The hosts exists in DNS but has no addresses)
       *   - SystemError::ecEAI_NONAME (The hosts does not exist in DNS)
       *
       * @param hostname The host name to lookup.
       * @param info The AddrInfo structure to fill.
       * @return SystemError::ecOK if the lookup resulted in at least one address.
       */
      static common::SystemError getHostAddrInfo( const std::string &hostname, AddrInfo& info );

      /**
       * Return the first hit on SocketParams in address and canonicalname.
       *
       * Could return the following SystemError
       *   - SystemError::ecOK
       *   - SystemError::ecEAI_ADDRFAMILY (The hosts exists but has not address in the requested address family)
       *   - SystemError::ecEAI_NODATA (The hosts exists in DNS but has no addresses)
       *   - SystemError::ecEAI_NONAME (The hosts does not exist in DNS)
       *
       * @param hostname The host name to lookup.
       * @param params The SocketParams the entry must match to.
       * @param address The result address (set to invalid when this call returns an error).
       * @param canonicalname The canonicalname of hostname (set to "" when this call returns an error).
       * @return SystemError::ecOK if the lookup resulted in an address.
       */
      static common::SystemError getHostAddrInfo( const std::string &hostname,
                                                  SocketParams &params,
                                                  Address &address,
                                                  std::string &canonicalname );

      /**
       * Do a reverse DNS lookup on this Address and return in hostname.
       *
       * Could return the following SystemError
       *   - SystemError::ecOK
       *   - SystemError::ecEAI_AGAIN
       *   - SystemError::ecEAI_BADFLAGS
       *   - SystemError::ecEAI_FAIL
       *   - SystemError::ecEAI_FAMILY
       *   - SystemError::ecEAI_MEMORY
       *   - SystemError::ecEAI_NONAME
       *   - SystemError::ecEAI_OVERFLOW
       *   - SystemError::ecEAI_SYSTEM is silently replaced by errno
       * @param hostname The string to receive the hostname.
       * @return The error or SystemError::ecOK on success.
       */
      common::SystemError getNameInfo( std::string &hostname ) const;

      /**
       * Do a reverse DNS lookup on this Address and its port, and return in hostname and service.
       * @param hostname The string variable to receive the hostname.
       * @param service The string variable to receive the service name.
       * @return The error or SystemError::ecOK on success.
       * @see getNameInfo() for a full list of possible errors returned.
       */
      common::SystemError getNameInfo( std::string &hostname, std::string &service ) const;

      /**
       * Do a reverse DNS lookup on this Address and its port, and return in hostname and port.
       * @param hostname The string variable to receive the hostname.
       * @param port The uint16_t variable to receive the port number.
       * @return The error or SystemError::ecOK on success.
       * @see getNameInfo() for a full list of possible errors returned.
       */
      common::SystemError getNameInfo( std::string &hostname, uint16_t &port ) const;

    private:

      /**
       * Initialize the internals.
       */
      void init();

      /**
       * Explicit cast of addr_ as sockaddr_in*.
       * @return The pointer.
       */
      struct sockaddr_in* asIPv4Address() const { return (struct sockaddr_in *)&addr_; };

      /**
       * Explicit cast of addr_ as sockaddr_in6*.
       * @return The pointer.
       */
      struct sockaddr_in6* asIPv6Address() const { return (struct sockaddr_in6 *)&addr_; };

      /**
       * The address storage (for either ipv4 or ipv6).
       */
      struct sockaddr_storage addr_;

    /**
     * Socket may access this class directly.
     */
    friend class Socket;
    /**
     * BaseSocket may access this class directly.
     */
    friend class BaseSocket;

  };


  /**
   * Address info item as used in AddrInfo
   * @see AddrInfo
   * @see Address::getHostAddrInfogetHostAddrInfo( const std::string &hostname, AddrInfo& info )
   * @see Address::getHostAddrInfo( const std::string &hostname, const SocketParams &params, Address &address, string &canonicalname )
   */
  struct AddrInfoItem {

    /**
     * The Address of this item.
     */
    Address address;

    /**
     * The SocketParams for this item.
     */
    SocketParams params;

    /**
     * String representation.
     * @return The string.
     */
    std::string asString() {
      std::stringstream ss;
      ss << address.asString() << " " << params.asString();
      return ss.str();
    };
  };

  /**
   * Address info, comprising the canonical name of a host, and list of address info items for the host.
   * @see AddrInfoItem
   * @see Address::getHostAddrInfo()
   */
  struct AddrInfo {

    /**
     * Initialize
     */
    AddrInfo() {
      items.clear();
      canonicalname = "";
    };

    /**
     * The canonical name for a host.
     */
    std::string canonicalname;

    /**
     * AddrInfoItem list for the host.
     */
    std::list<AddrInfoItem> items;
  };

};

#endif
