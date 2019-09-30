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
 * @file sslcontext.hpp
 * Defines the arca::network::SSLContext class.
 */

#ifndef network_sslcontext_hpp
#define network_sslcontext_hpp

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <common/exception.hpp>
#include <common/systemerror.hpp>

#include <string>

namespace dodo::network {

  using namespace std;

  /**
   * SSL encryption context, typically a private/publick keypair.
   */
  class SSLContext : public dodo::common::DebugObject {
    public:
      SSLContext();
      virtual ~SSLContext();

      /**
       * Load certificate and private key.
       * @param certfile The public identity
       * @param keyfile The private key
       */
      common::SystemError loadCertificates( const string& certfile, const string& keyfile );

      /**
       * Set a list of ciphers the SSLContext will accept
       */
      void setCipherList( const string& cipherlist );

      /**
       * Set SSL options
       */
      long setOptions( long option );

      /**
       * Return a pointer to the SSL_CTX
       * @return a pointer to the SSL_CTX
       */
      SSL_CTX* getContext() const { return  sslctx_; };

      /**
       * Write ssl errors occured in this thread to ostream, and clear their error state.
       * @return The number of SSL errors written.
       */
      size_t writeSSLErrors( ostream& out, char terminator = 0 );

      /**
       * Get all SSL errors as a single string, and clear their error state.
       * @param terminator The terminator character for a single error line. If 0, no character will be appended.
       * @see writeSSLErrors( ostream& out)
       */
      string getSSLErrors( char terminator = 0 );

    private:
      /**
       * Initialize the SSL library
       */
      static void InitializeSSL();

      /**
       * Shutdown the SSL library
       */
      static void ShutdownSSL();

      /**
       * The openssl SSL_CTX
       */
      SSL_CTX* sslctx_;

      /**
       * Needs acces to InitializeSSL
       */
      friend void init();

      /**
       * Needs acces to ShutdownSSL
       */
      friend void shutdown();
  };

}

#endif
