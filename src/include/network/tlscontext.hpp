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

#include <iostream>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <string>

#include "common/config.hpp"
#include "common/exception.hpp"
#include "common/systemerror.hpp"



namespace dodo::network {

  /**
   * TLS security context defines the environment for TLSSocket. A TLSContext specifices (or can specify)
   *
   *   - Configuring trust (either system or custom)
   *   - Set minimum TLS version required
   *   - Define peer verfiication method, (dis)allow SAN wildcards, enable SNI
   *   - Specify an identity by PEM or PKCS12
   *   - limit negotiated ciphers
   *
   * A single TLSContext can be shared among multiple TLSSocket classes. As a TLSContext is a common deployment configuration
   * artifact, there is TLSContext( const Config& config, const KeyPath& path ) to initialize.
   *
   * A TLSContext in client perepective may need to support a variety of TLS servers and thus be more lenient in the minimum
   * TLS version and or ciphers negotiated. A local scope server can be hardened by limiting TLS versions to TLS 1.3 and
   * configuring just the strongest cipher.
   *
   * See @ref developer_networking for more information on the role of this class.
   *
   * @todo Implement the OCSP protocol for revoked cert check
   * @see http://www.zedwood.com/article/cpp-x509-certificate-revocation-check-by-ocsp
   * @see https://stackoverflow.com/questions/56253312/how-to-create-ocsp-request-using-openssl-in-c
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
       *
       * | PeerVerification | SSL_CTX_set_verify                                  |
       * | -----------------| ----------------------------------------------------|
       * | pvVerifyNone     | SSL_VERIFY_NONE                                     |
       * | pvVerifyPeer     | SSL_VERIFY_PEER \| SSL_VERIFY_FAIL_IF_NO_PEER_CERT  |
       * | pvVerifyFQDN     | SSL_VERIFY_PEER \| SSL_VERIFY_FAIL_IF_NO_PEER_CERT  |
       *
       * | PeerVerification | Server / accept | Client / connect |
       * |------------------|-----------------|------------------|
       * | pvVerifyNone     | encryption of traffic |  encryption of traffic |
       * | pvVerifyPeer     | pvVerifyNone + client must present trusted cert |  pvVerifyNone + server must present trusted cert |
       * | pvVerifyFQDN     | pvVerifyPeer |  pvVerifyPeer + X509Certificate::verifyName() |
       *
       * @see https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_verify.html
       */
      enum class PeerVerification {
        pvVerifyNone,           /**< No peer verification - transmission is encrypted, peer is accepted even if
                                     peer certificate is invalid - so any peer can read all data sent.*/
        pvVerifyPeer,           /**< The peer must have a trusted certificate (unless a anonymous cipher is used). */
        pvVerifyFQDN,           /**< As pvVerifyPeer, but the remote DNS name must match either the peer cert commonname
                                     or match one of the peer cert subjectAltNames */
      };

      /**
       * Construct a TLS context. Use depends on the context being either server or client side, see TLSContext::PeerVerification.
       *
       * Example for a server-side setup enforcing at least TLS1.3, requiring the peer to present a trusted certificate (pvVerifyPeer).
       * ```C
       * TLSContext tlscontext( PeerVerification::pvVerifyPeer,
      *                         TLSContext::TLSVersion::tls1_3,
                                false,                           // server-side does not need SNI
                                false );                         // server-side does not do SAN matching
         tlscontext.loadPEMIdentity( "server.crt", "server.key", "passphrase" );
       * ```
       * @param peerverficiation The TLSContext::PeerVerification method to use.
       * @param tlsversion The TLS version to use. Use of default is less future code hassle.
       * @param enableSNI Enable the Server Name Indication extension. Note that this exposes the target hostname
       * @param allowSANWildcards Allow SAN wildcard matching under pvVerifyFQDN
       * of TLSSocket connections as the hostname is sent unencrypted, facilitated all kinds of evil such as
       * censorship. Use only when you must connect to a server that requires it.
       *
       * @see https://en.wikipedia.org/wiki/Server_Name_Indication
       */
      TLSContext( const PeerVerification& peerverficiation = PeerVerification::pvVerifyFQDN,
                  const TLSVersion& tlsversion = TLSVersion::tlsBest,
                  bool  enableSNI = true,
                  bool allowSANWildcards = true );

      /**
       * Construct a TLSContext from a dodo::common::Config as a dodo::common::Config::KeyPath.
       * ```YAML
       * tlscontext:
       *   peer-verification: pvVerifyFQDN                    # mandatory
       *   tls-version: 1.3                                   # mandatory
       *   enable-sni: true                                   # mandatory
       *   allow-san-wildcards: true                          # mandatory
       *   trust:                                             # optional
       *     file: <path to PEM file>                         # at least one of file or path
       *     path: <path to directory with PEM files>         # at least one of file or path
       *   ciphers:                                           # mandatory
       *     - TLS_AES_256_GCM_SHA384                         # at least one entry
       *     - TLS_AES_128_GCM_SHA256
       * ```
       * If the PEM format is used to provide keys and passphrase:
       * ```YAML
       * tlscontext:
       *   pem:
       *     private: <path to private key PEM>               # mandatory
       *     public: <path to public key PEM>                 # mandatory
       *     passphrase: <ENC[...]>                           # mandatory
       * ```
       * If the PKCS12 format is used to provide keys and passphrase:
       * ```YAML
       * tlscontext:
       *   pkcs12:
       *     file: <path to PKCS12 file>                      # mandatory
       *     passphrase: <ENC[...]>                           # mandatory
       * ```
       * The tlscontext node can have any name and appear anywhere in the YAML file
       * ```YAML
       * dodo:
       *   common:
       *     ...
       * my-server:
       *   tls:
       *     peer-verification: pvVerifyFQDN
       *     ...
       *
       * ```
       * in which case the TLSContext can be read as
       * ```C
       * common::Config *config = common::Config::initialize( "my-config.yaml" );
       * network::TLSContext tlscontext( *config, {"my-server","tls"} );
       * ```
       *
       * @param config The dodo::common::Config to read from.
       * @param path The KeyPath to the root of the TLSContext
       */
      TLSContext( const common::Config& config, const common::Config::KeyPath& path );


      virtual ~TLSContext();

      /**
       * Load a certificate and the corresponding private key for an identity.
       * @param certfile The certificate PEM file.
       * @param keyfile The private key PEM file.
       * @param passphrase The passphrase for the private key PEM file. If the private key is not protected by a
       * passphrase its value is stored in this object nonetheless but unused.
       */
      void loadPEMIdentity( const std::string& certfile,
                            const std::string& keyfile,
                            const std::string& passphrase );

      /**
       * Loads a private key, matching certificate and optional CA certificates (eg a truststore) from a
       * PKCS12 file.
       * @param p12file The PKCS12 file to read from.
       * @param p12passphrase The passphrase for the PKCS12 file.
       */
      void loadPKCS12( const std::string &p12file,
                       const std::string &p12passphrase );

      /**
       * Set a list of ciphers, separated by a colon, the TLSContext will accept. This overrides the default
       * behavior of opennsel. Use setCipherList() for < TLS1.3 and setCipherSuite for TLS1.3 and higher.
       *
       * When the TLSContext is used in a client that must connect to a variety of servers, the acceptable ciphers
       * likely need to be more lenient, but local-scope servers may limit to the strongest cipher available by
       * overriding.
       *
       * @code
       * context.setCipherList( "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384" );
       * @endcode
       *
       * @see https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_cipher_list.html
       * @see https://www.openssl.org/docs/man1.1.1/man1/ciphers.html
       *
       * A list of available ciphers is given by
       * ```
       * $ openssl ciphers -tls1_2 -s
       * $ openssl ciphers -tls1_3 -s
       * ```
       *
       * Note that this call will not return a SystemError, but throws a dodo::common::Exception when the cipher list
       * is invalid.
       *
       * @param cipherlist The cipherlist.
       * @throw dodo::common::Exception
       */
      void setCipherList( const std::string& cipherlist );

      /**
       * Set a suite of ciphers, separated by a colon, the TLSContext will accept for TLV1.3 handshakes. This overrides
       * the default behavior of opennsel. Use setCipherList() for < TLS1.3 and setCipherSuite for TLS1.3 and higher.
       *
       * When the TLSContext is used in a client that must connect to a variety of servers, the acceptable ciphers
       * likely need to be more lenient, but local-scope servers may limit to the strongest cipher available by
       * overriding.
       *
       * @code
       * context.setCipherList( "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384" );
       * @endcode
       *
       * @see https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_cipher_list.html
       * @see https://www.openssl.org/docs/man1.1.1/man1/ciphers.html
       *
       * A list of available ciphers is given by
       * ```
       * $ openssl ciphers -tls1_2 -s
       * $ openssl ciphers -tls1_3 -s
       * ```
       *
       * Note that this call will not return a SystemError, but throws a dodo::common::Exception when the cipher suite
       * is invalid.
       *
       * @param ciphersuite The ciphersuite.
       * @throw dodo::common::Exception
       */
      void setCipherSuite( const std::string& ciphersuite );

      /**
       * Set SSL options
       * @param option The option or OR-ed options to apply.
       * @return The options applied.
       * @see https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_options.html
       */
      long setOptions( long option );

      /**
       * Trust all certificates (PEM format) in the specified file and/or directory.
       * @param cafile A PEM file containing one or more certificates. If an empty string, unused.
       * @param capath A directory containing certificate files. If an empty string, unused.
       */
      void setTrustPaths(  const std::string& cafile,
                           const std::string& capath );

      /**
       * Return a pointer to the SSL_CTX
       * @return a pointer to the SSL_CTX
       */
      SSL_CTX* getContext() const { return  tlsctx_; };

      /**
       * Return the getPeerVerification mode.
       * @return the getPeerVerification.
       */
      PeerVerification getPeerVerification() const { return peerverficiation_; }

      /**
       * Return true when SNI (server Name Information) is to be enabled by TLSSocket objects using this TLSContext.
       * @return true when SNI is enabled.
       */
      bool isSNIEnabled() const { return enable_sni_; }

      /**
       * If true, TLS will allow SAN wildcard matching.
       * @return True if wildcard matching on the SAN is allowed.
       */
      bool isAllowSANWildcards() const { return allow_san_wildcards_; }


      /**
       * Get a PeerVerfication enum from a string. The comparison is case sensitive and must match the enum name
       * ( "pvVerifyNone", "pvVerifyPeer", "pvVerifyFQDN" ).
       * If the name does not translate, a common::Exception is thrown.
       * @param src The source string.
       * @return the PeerVerification.
       */
      static PeerVerification peerVerficiationFromString( const std::string &src );

      /**
       * Convert the src string to a TLSVersion or throw a common::Exception if that mapping fails. TLSversion strings
       * could be "1.1", "1.2" and "1.3".
       * @param src The source string.
       * @return The TLSVersion specified by the string.
       */
      static TLSVersion tlsVersionFromString( const std::string &src );

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
       * Construct the TLSContext.
       * @param peerverficiation The TLSContext::PeerVerification method to use.
       * @param tlsversion The TLS version to use. Use of default is less future code hassle.
       * @param enableSNI Enable the Server Name Indication extension. Note that this exposes the target hostname
       * @param allowSANWildcards Allow SAN wildcard matching under pvVerifyFQDN
       */
      void construct( const PeerVerification& peerverficiation,
                      const TLSVersion& tlsversion,
                      bool  enableSNI,
                      bool allowSANWildcards );

      /**
       * Password callback, returns the passphrase set in the TLS context by the passphrase argument of
       * loadCertificate or pkeypassphrase argument of the loadPKCS12 method.
       * @param buf The passphrase should be copied to here.
       * @param size No more than size bytes should be copied into buf.
       * @param rwflag 0 = decryption 1 = encryption
       * @param userdata Pass a pointer to the TLSContext object.
       * @return the character length of the passphrase string.
       */
      static int pem_passwd_cb( char *buf, int size, int rwflag, void *userdata );

      /**
       * The openssl SSL_CTX
       */
      SSL_CTX* tlsctx_;

      /**
       * The TLS version
       */
      TLSVersion tlsversion_;

      /**
       * The peer verification method used.
       */
      PeerVerification peerverficiation_;

      /**
       * The passphrase to decrypt encrypted private keys (may be empty when the key is not encrypted).
       */
      std::string passphrase_;

      /**
       * Enable / disable SNI on TLSSocket objects using this TLSContext.
       */
      bool enable_sni_;

      /**
       * Allow SAN names to match agains wildcards (eg foo.domain.org matches *.domain.org).
       */
      bool allow_san_wildcards_;

      /**
       * Enable / disable CRL (Certificate Revocation List) checking.
       */
      bool enable_clr_;


  };

}

#endif
