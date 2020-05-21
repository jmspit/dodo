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
 * @file x509cert.hpp
 * Defines the dodo::network::SSLSocket class.
 */

#ifndef network_x509cert_hpp
#define network_x509cert_hpp

#include <openssl/ssl.h>
#include <string>

namespace dodo::network {

  /**
   * Interface common to X509 documents.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509Common {
    public:

      /**
       * Enumaration of X509 document types.
       */
      enum class X509Type {
        Unknown,                    /**< Unknown PEM document */
        PrivateKey,                 /**< Private key PEM document (possibly encrypted). */
        PublicKey,                  /**< Public key PEM document */
        CertificateSigningRequest,  /**< CSR PEM document */
        Certificate                 /**< Certificate PEM document */
      };

      /**
       * Detects a X509 document type from a PEM file.
       * The PEM is not checked on validity, and a result other than X509Type::Unknown does not imply the document
       * is well formed and valid. Note that both private key and encrypted private key PEM files are identified as
       * X509Type::PrivateKey.
       * @param file The file name to be content-checked.
       * @param tag Receives the PEM tag (eg 'CERTIFICATE','PRIVATE KEY',..).
       * @return the X509Type.
       */
      static X509Type detectX509Type( const std::string file, std::string &tag );

    protected:
      /**
       * Convert the data contents of a BIO to a std::string.
       * @param bio The source BIO.
       * @return The string representation of the BIO contents.
       */
      static std::string bio2String( BIO* bio );

    private:
      /** Never construct, interface class. */
      X509Common() = delete;
      /** Never destruct, interface class. */
      ~X509Common() = delete;
  };

  /**
   * X509 Certificate signing request (CSR) interface. Note that this is an interface class, it does not
   * manage ownership of X509_REQ structures.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509CertificateSigningRequest : public X509Common {
    public:

      /**
       * Load a certificate signing request (CSR) from a PEM file. The X509_REQ object pointed to gets owned by
       * the caller and must be freed with free( X509_REQ* cert ). Note that the call will fail if the file is
       * not a CSR, even if it is a valid PEM  file - such as a certifcate or a private key in
       * PEM format.
       *
       * @param file The filename to load from.
       * @throw throws a common::Execption when the call fails to produce.
       * @return Pointer to your X509_REQ.
       */
      static X509_REQ* loadPEM( const std::string file );

      /**
       * Free / clean an X509 object.
       * @param cert The X509_REQ object to free.
       * @return nothing
       */
      static void free( X509_REQ* cert ) { X509_REQ_free( cert ); }

      /**
       * Get the CSR subject.
       * @param cert The source CSR / X509_REQ.
       * @return the CSR subject.
       */
      static std::string getSubject( const X509_REQ *cert );

      /**
       * Get the certificate fingerprint in string format,
       * multiple hexadecimal bytes values sperated by a colon.
       * @param cert The source CSR / X509_REQ.
       * @param hashname The name of the hash algorithm to use (eg 'md5','sha1','sha256','sha384',..). Defaults
       * to 'sha256'.
       * @return A string representation of the fingerprint.
       */
      static std::string getFingerPrint( const X509_REQ *cert, const std::string hashname = "sha256" );


    private:
      /** Never construct, interface class. */
      X509CertificateSigningRequest() = delete;
      /** Never destruct, interface class. */
      ~X509CertificateSigningRequest() = delete;
  };

  /**
   * X509 public key certificate (PKC) interface. Note that this is an interface class, it does not
   * manage ownership of X509 structures.
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
       * @param file The PEM file to load from.
       * @throw throws a common::Execption when the call fails to produce.
       * @return Pointer to the X509 document.
       */
      static X509* loadPEM( const std::string file );

      /**
       * Free / clean an X509 object.
       * @param cert A pointer to the X509 certificate.
       * @return nothing
       */
      static void free( X509* cert ) { X509_free( cert ); }

      /**
       * Get the certificate issuer.
       * @param cert A pointer to the X509 certificate.
       * @return the issuer string.
       */
      static std::string getIssuer( const X509 *cert );

      /**
       * Get the certificate serial number as concatenated hex bytes. Note that the serial number is only supposed to be unique
       * among certificates signed by a single CA. To truly identify certificates, use getFingerPrint().
       * @param cert The source PKC / X509.
       * @return the serial string.
       */
      static std::string getSerial( const X509 *cert );

      /**
       * Get the certificate subject.
       * @param cert A pointer to the X509 certificate.
       * @return the subject string.
       */
      static std::string getSubject( const X509 *cert );

      /**
       * Get the certificate fingerprint in string format,
       * multiple hexadecimal bytes values sperated by a colon.
       * @param cert A pointer to the X509 certificate.
       * @param hashname The name of the hash algorithm to use (eg 'md5','sha1','sha256','sha384',..). Defaults
       * to 'sha256'.
       * @return A string representation of the finherprint.
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
