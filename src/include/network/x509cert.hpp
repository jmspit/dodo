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
   * Functionality common to X509 documents.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509Common {
    public:
      enum class X509Type {
        Unknown,
        PrivateKey,
        PublicKey,
        CertificateSigningRequest,
        Certificate
      };

      /**
       * Detects a X509 document type from a PEM file.
       * The PEM is not checked on validity, and a result other than PrivateKey::UnKnown does not imply the document
       * is well formed and valid. Note that both private key and encrypted private key PEM files are identified as
       * X509Type::PrivateKey.
       */
      static X509Type detectX509Type( const std::string file, std::string &tag );

    protected:
      /**
       * Convert the data contents of a BIO to a std::string.
       */
      static std::string bio2String( BIO* bio );
    private:
      /** Never construct, interface class. */
      X509Common() = delete;
      /** Never destruct, interface class. */
      ~X509Common() = delete;
  };

  /**
   * An X509 Certificate signing request interface.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509CertificateSigningRequest : public X509Common {
    public:
      static X509_REQ* loadPEM( const std::string file );

      /**
       * Free / clean an X509 object.
       */
      static void free( X509_REQ* cert ) { X509_REQ_free( cert ); }

      /**
       * Get the CSR subject.
       */
      static std::string getSubject( const X509_REQ *cert );

      /**
       * Get the certificate fingerprint in string format,
       * multiple hexadecimal bytes values sperated by a colon.
       */
      static std::string getFingerPrint( const X509_REQ *cert, const std::string hashname = "sha256" );


    private:
      /** Never construct, interface class. */
      X509CertificateSigningRequest() = delete;
      /** Never destruct, interface class. */
      ~X509CertificateSigningRequest() = delete;
  };

  /**
   * An X509 Certificate aka public key certificate.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509Certificate : public X509Common {
    public:

      /**
       * Load a public key certificate (aka 'certificate') from a PEM file. The X509 object pointed to gets owned by
       * the caller and must be freed when done with free( X509* cert ). Note that the call will fail if the file is
       * not a public key certificate, even though it is a valid PEM  file - such as a CSR or a private key in
       * PEM format.
       *
       * @throw throws a common::Execption when the call fails to produce.
       */
      static X509* loadPEM( const std::string file );

      /**
       * Free / clean an X509 object.
       */
      static void free( X509* cert ) { X509_free( cert ); }

      /**
       * Get the certificate issuer.
       */
      static std::string getIssuer( const X509 *cert );

      /**
       * Get the serial number.
       */
      static std::string getSerial( const X509 *cert );

      /**
       * Get the certificate subject.
       */
      static std::string getSubject( const X509 *cert );

      /**
       * Get the certificate fingerprint in string format,
       * multiple hexadecimal bytes values sperated by a colon.
       */
      static std::string getFingerPrint( const X509 *cert, const std::string hashname = "sha256" );

    private:
      /** Never construct, interface class. */
      X509Certificate() = delete;
      /** Never destruct, interface class. */
      ~X509Certificate() = delete;
  };

}

#endif
