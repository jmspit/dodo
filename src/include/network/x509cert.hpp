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
#include <openssl/x509v3.h>
#include <string>
#include <list>
#include <map>

namespace dodo::network {

  /**
   * Interface common to X509 documents.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509Common {
    public:

      /**
       * The SubjectAltName type.
       */
      enum class SANType {
        stDNS   = GEN_DNS,    /**< A valid DNS name such as myhost.mydomain.org */
        stURI   = GEN_URI,    /**< A valid URI */
        stEMAIL = GEN_EMAIL,  /**< A valid email address */
        tsIP    = GEN_IPADD   /**< A valid IPv4 or IPv6 address */
      };

      /**
       * Subject AltName record.
       */
      struct SAN {
        /**
         * The type.
         */
        X509Common::SANType san_type;

        /**
         * The name.
         */
        std::string san_name;
      };


      /**
       * Attributes that together constitute a X509 identity.
       */
      struct Identity {

        Identity() : countryCode(""), state(""), locality(""), organization(""), organizationUnit(""),
          commonName(""),email("") {}

        /**
         * A two-character country code, for example NL for The Netherlands.
         */
        std::string countryCode;

        /**
         * The State or Province name.
         */
        std::string state;

        /**
         * The locality name (city, town).
         */
        std::string locality;

        /**
         * The organization name.
         */
        std::string organization;

        /**
         * The organizational unit name.
         */
        std::string organizationUnit;

        /**
         * The common name.
         */
        std::string commonName;

        /**
         * The email address.
         */
        std::string email;

        /**
         * The businessCategory
         */
        std::string businessCategory;

        /**
         * The jurisdiction country code.
         */
        std::string jurisdictionC;

        /**
         * The jurisdiction state.
         */
        std::string jurisdictionST;

        /**
         * A cert serial number.
         */
        std::string serialNumber;

        /**
         * The street address.
         */
        std::string street;

        /**
         * The postal code.
         */
        std::string postalCode;

        /**
         * Other key-value pairs in the identity.
         */
        std::map<std::string,std::string> other;
      };

      /**
       * Enumeration of X509 document types.
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
       * Parse a subject or issuer string into an Identity.*
       * @param src The identity string
       * @return the Identity.
       */
      static Identity parseIdentity( const std::string src );

    private:
      /** Never construct, interface class. */
      X509Common() = delete;
      /** Never destruct, interface class. */
      ~X509Common() = delete;
  };


  /**
   * Serialize an Identity.
   * @param out The stream to write to.
   * @param identity The identity to write.
   * @return A reference to the stream written to.
   */
  //inline std::ostream & operator<<( std::ostream &out, const X509Common::Identity& identity ) {
    //out << std::string("C=") << identity.countryCode;
    //out << std::string(",ST=") << identity.state;
    //out << std::string(",L=") << identity.locality;
    //out << std::string(",O=") << identity.organization;
    //out << std::string(",OU=") << identity.organizationUnit;
    //out << std::string(",CN=") << identity.commonName;
    //out << std::string(",emailAddress=") << identity.email;
    //for ( auto o : identity.other ) {
      //out << "," << o.first << "=" << o.second;
    //}
    //return out;
  //}

  /**
   * X509 Certificate signing request (CSR) interface. Note that this is an sinterface class, it does not
   * manage ownership of X509_REQ structures.
   *
   * See @ref developer_networking for details on the role of this class.
   */
  class X509CertificateSigningRequest : public X509Common {
    public:

      /**
       * Load a certificate signing request (CSR) from a PEM file. The X509_REQ object pointed to gets owned by
       * the caller and must be freed with free( X509_REQ* cert ). Note that the call will fail if the file is
       * not a CSR, even if it is a valid PEM  file - such as a certificate or a private key in
       * PEM format.
       *
       * @param file The filename to load from.
       * @throw common::Exception when the openSSL BIO fails to create.
       * @throw common::Exception when the file cannot be read.
       * @throw common::Exception when the file is not a valid PEM file.
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
       * Get the CSR subject identity.
       * @param cert The source CSR / X509_REQ.
       * @return the CSR subject identity.
       */
      static X509Common::Identity getSubject( const X509_REQ *cert );

      /**
       * Get the certificate fingerprint (a hash on the public key modulus) in string format,
       * multiple hexadecimal bytes values separated by a colon.
       * `openssl list -digest-algorithms` shows a full list of hash (digest) names. Stick to newer hash algorithms
       * from the SHA-3 family.
       *
       * @throws common::Exception if the digest name is invalid.
       *
       * @see https://en.wikipedia.org/wiki/Secure_Hash_Algorithms
       *
       * @param cert A pointer to the X509 certificate.
       * @param hashname The name of the hash algorithm to use. Defaults to 'shake256'. Names are case-insensitive.
       * @return A string representation of the fingerprint.
       */
      static std::string getFingerPrint( const X509_REQ *cert, const std::string hashname = "shake256" );


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
       * not a public key certificate, even though it is a valid PEM  file. Also note that the call will return ony the
       * first certificate if the PEM file contains multiple certificates.
       * @param file The PEM file to load from.
       * @throw common::Exception when the openSSL BIO fails to create.
       * @throw common::Exception when the file cannot be read.
       * @throw common::Exception when the file is not a valid PEM file.
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
      static X509Common::Identity getIssuer( const X509 *cert );

      /**
       * Get the certificate serial number as concatenated hex bytes. Note that the serial number is only supposed to be unique
       * among certificates signed by a single CA. To truly identify certificates, use getFingerPrint().
       * @param cert The source PKC / X509.
       * @return the serial string.
       */
      static std::string getSerial( const X509 *cert );

      /**
       * Get the certificate subject identity.
       * @param cert A pointer to the X509 certificate.
       * @return the subject identity.
       */
      static X509Common::Identity getSubject( const X509 *cert );

      /**
       * Get the SAN (subject alternate name) list for the certificate, which may be empty.
       * @param cert A pointer to the X509 certificate.
       * @return A list of SAN.
       */
      static std::list<X509Common::SAN> getSubjectAltNames( const X509* cert );

      /**
       * Get the certificate fingerprint (a hash on the public key modulus) in string format,
       * multiple hexadecimal bytes values separated by a colon.
       * `openssl list -digest-algorithms` shows a full list of hash (digest) names. Stick to newer hash algorithms
       * from the SHA-3 family.
       *
       * @throws common::Exception if the digest name is invalid.
       *
       * @see https://en.wikipedia.org/wiki/Secure_Hash_Algorithms
       *
       * @param cert A pointer to the X509 certificate.
       * @param hashname The name of the hash algorithm to use. Defaults to 'shake256'. Names are case-insensitive.
       * @return A string representation of the fingerprint.
       */
      static std::string getFingerPrint( const X509 *cert, const std::string hashname = "shake256" );

      /**
       * Verify a peer name against this certificate's CN and SubjectAltnames.
       *
       * The name is always matched against the CN, regardless of the X509Common::SANType.
       * The name is matched against SubjectAltNames that match the X509Common::SANType.
       *
       * @param cert A pointer to the X509 certificate.
       * @param san The SAN structure to compare against.
       * @return true if the name matches.
       */
      static bool verifySAN( const X509 *cert, const SAN &san );

    private:

      /**
       * Verify a peer name matches a SAN.
       * @param peer The name of the peer.
       * @param san The SubjectAltname of the peer.
       * @param allowwildcard If true, allow wildcards.
       * @return true when the name matches.
       */
      static bool verifyName( const std::string peer, const std::string san, bool allowwildcard = false );

      /**
       * Verify a peer IP matches a SAN of type stIP. The strings are converted to IP addresses, both must be valid IP
       * addresses and they must be equal ( Address::operator==( const Address &) ).
       * @param peer The ipv4 or ipv6 of the peer (as a string).
       * @param san The ipv4 or ipv6 SubjectAltname of the peer (as a string).
       * @return true when the IP matches.
       */
      static bool verifyIP( const std::string peer, const std::string san );

      /** Never construct, interface class. */
      X509Certificate() = delete;
      /** Never destruct, interface class. */
      ~X509Certificate() = delete;
  };

}

#endif
