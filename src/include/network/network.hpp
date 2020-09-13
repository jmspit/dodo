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
 * @file network.hpp
 * Includes network headers and implements network::initLibrary() and network::closeLibrary().
 */

#ifndef network_network_hpp
#define network_network_hpp

#include "network/address.hpp"
#include "network/basesocket.hpp"
#include "network/protocol/stomp.hpp"
#include "network/socket.hpp"
#include "network/tcplistener.hpp"
#include "network/tcpserver.hpp"
#include "network/tlscontext.hpp"
#include "network/tlssocket.hpp"
#include "network/x509cert.hpp"

namespace dodo {

  /**
   * Interface for network communication. See @ref developer_networking for details.
   */
  namespace network {

    /**
     * Initialize the dodo::network library.
     */
    void initLibrary() {
      OPENSSL_init_crypto( 0, nullptr );
      OPENSSL_init_ssl( 0, nullptr );

      //SSL_load_error_strings();
      //SSL_library_init();
      //OpenSSL_add_all_algorithms();
      //OPENSSL_cpuid_setup();
      //OpenSSL_add_all_ciphers();
      //OpenSSL_add_all_digests();
    }

    /**
     * Close the dodo::network library.
     */
    void closeLibrary() {
      //ERR_free_strings();
      OPENSSL_cleanup();
    }

    /**
     * Application layer protocols.
     */
    namespace protocol {

      /**
       * The STOMP 1.2 protocol (earlier versions not supported).
       */
      namespace stomp {
      }
    }

  }

}

#endif