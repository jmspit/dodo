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
 * @file tlscontext.hpp
 * Defines the dodo::network::TLSContext class.
 */

#ifndef network_tlscontext_hpp
#define network_tlscontext_hpp

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <common/exception.hpp>
#include <common/systemerror.hpp>

#include <string>

namespace dodo::network {

  /**
   * TLS security context.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class TLSContext : public dodo::common::DebugObject {
    public:

      /**
       * The TLS version. Use tlsBest.
       */
      enum class TLSVersion {
        tls1_1,  /**< TLS 1.1 disables SSLv2, SSLv3 and TLS 1.0*/
        tls1_2,  /**< TLS 1.2 disables SSLv2, SSLv3, TLS 1.0 and TLS 1.1 */
        tls1_3,  /**< TLS 1.3 disables SSLv2, SSLv3, TLS 1.0, TLS 1.1 and TLS 1.2 */
        tlsBest = tls1_3 /**< Use as default TLS version*/
      };

      /**
       * The TLS peer verification method.
       */
      enum class PeerVerification {
        pvNone,           /**< No peer verification. */
        pvCA,             /**< Based on the DNS FQDN verified by a trusted 3rd party to be the owner of a private key. */
        pvCustom          /**< Custom peer verification. */
      };

      /**
       * Construct a TLS context.
       * @param tlsversion The TLS verion to use. Use of default is less future code hassle.
       */
      TLSContext( const TLSVersion& tlsversion = TLSVersion::tlsBest );


      virtual ~TLSContext();

      /**
       * Load certificate and private key.
       * @param certfile The public identity
       * @param keyfile The private key
       * @param passphrase The passphrase for the private key. If the private key is not protected by a passphrase,
       * this value is ignored.
       */
      void loadCertificates( const std::string& certfile, const std::string& keyfile, const std::string passphrase );

      /**
       * Set a list of ciphers the TLSContext will accept. An example semicolon-separated cipher list might be
       * "DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256", both TLS cyphers TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
       * and TLS_DHE_RSA_WITH_AES_128_GCM_SHA256. Another example is "HIGH:!aNULL:!MD5" which disables
       * no-authentication ciphers, disables MD5 ciphers and requires algorithms with at least 128bit keys (at time of
       * this writing, see url below).
       *
       * Note that this call will not return a SystemError, but throw a dodo::common::Exception on failure, a mistake
       * here means a faulty deployment/configuration, and that should not continue under any circumstance.
       * @param cipherlist The cipherlist.
       * @throw dodo::common::Exception
       * @see http://openssl.cs.utah.edu/docs/apps/ciphers.html
       */
      void setCipherList( const std::string& cipherlist );

      /**
       * Set SSL options
       * @param option The option or OR-ed options to apply.
       * @return The options applied.
       */
      long setOptions( long option );

      /**
       * Return a pointer to the SSL_CTX
       * @return a pointer to the SSL_CTX
       */
      SSL_CTX* getContext() const { return  sslctx_; };

      /**
       * Write ssl errors occured in this thread to ostream, and clear their error state.
       * @param out The std::ostream to write to.
       * @param terminator The char to use to separate lines.
       * @return The number of SSL errors written.
       */
      static size_t writeSSLErrors( std::ostream& out, char terminator = 0 );

      /**
       * Get all SSL errors as a single string, and clear their error state.
       * @param terminator The terminator character for a single error line. If 0, no character will be appended.
       * @return The string.
       * @see writeSSLErrors( ostream& out)
       */
      static std::string getSSLErrors( char terminator = 0 );

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
       * The TLS version
       */
      TLSVersion tlsversion_;

      /**
       * The passphrase to decrypt encrypted private keys (may be empty when the key is not encrypted).
       */
      std::string passphrase_;


  };

}

#endif
