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
 * @file sslcert.hpp
 * Defines the dodo::network::SSLSocket class.
 */

#ifndef network_x509cert_hpp
#define network_x509cert_hpp

#include <openssl/ssl.h>
#include <string>

namespace dodo::network {

  /**
   * An X509 Certificate.
   */
  class X509Certificate {
    public:

      /**
       * Load a certificate from a PEM file. The X509 object pointed to gets owned by the caller and must be freed
       * when done.
       *
       * @see void free( X509* cert )
       */
      static X509* loadPEM( const std::string file );

      /**
       * Free / clean an X509 object.
       */
      static void free( X509* cert ) { X509_free( cert ); }

      /**
       * Get the certificate subject.
       */
      static std::string getSubject( const X509 *cert );

    protected:

      /**
       * Convert the data contents of a BIO to a string.
       */
      static std::string bio2String( BIO* bio );

    private:
      /** Never construct, interface class. */
      X509Certificate() = delete;
      /** Never destruct, interface class. */
      ~X509Certificate() = delete;
  };

}

#endif
