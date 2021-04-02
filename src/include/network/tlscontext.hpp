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

#include "common/exception.hpp"
#include "common/systemerror.hpp"



namespace dodo::network {

  /**
   * TLS security context. A single TLSContext can be shared among multiple TLSSocket classes.
   *
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
       * | pvVerifyCustom   | SSL_VERIFY_PEER \| SSL_VERIFY_FAIL_IF_NO_PEER_CERT  |
       *
       * | PeerVerification | Server / accept | Client / connect |
       * |------------------|-----------------|------------------|
       * | pvVerifyNone     | encryption of traffic |  encryption of traffic |
       * | pvVerifyPeer     | pvVerifyNone + client must present trusted cert |  pvVerifyNone + server must present trusted cert |
       * | pvVerifyFQDN     | pvVerifyPeer |  pvVerifyPeer + X509Certificate::verifyName() |
       * | pvVerifyCustom   | pvVerifyPeer + custom |  pvVerifyPeer + custom |
       *
       * @see https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_verify.html
       */
      enum class PeerVerification {
        pvVerifyNone,           /**< No peer verification - transmission is encrypted, peer cis accepted even if
                                     peer certificate is invalid and can read all data sent.*/
        pvVerifyPeer,           /**< The peer must have a trusted certificate (unless a anonymous cipher is used). */
        pvVerifyFQDN,           /**< As pvVerifyPeer, but the remote DNS name must match either the peer cert commonname
                                     or match one of the peer cert subjectAltNames */
        pvVerifyCustom,         /**< As pvVerifyPeer, but with custom certificate validation logic.*/
      };

      /**
       * Construct a TLS context.
       * @param peerverficiation The PeerVerification method to use.
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
       * Set a list of ciphers the TLSContext will accept. There are differences between TLSVersion tough,
       *
       * A few examples (note the hyphens and underscores)
       *
       *   - TLS 1.3 TLS_AES_256_GCM_SHA384
       *   - TLS 1.2 DHE-RSA-AES256-GCM-SHA384
       *   - TLS 1.1 DHE-RSA-AES256-GCM-SHA384
       *
       * @see https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_cipher_list.html
       *
       * Note that this call will not return a SystemError, but throws a dodo::common::Exception when the cipher list
       * is invalid.
       *
       * @param cipherlist The cipherlist.
       * @throw dodo::common::Exception
       * @see http://openssl.cs.utah.edu/docs/apps/ciphers.html
       */
      void setCipherList( const std::string& cipherlist );

      /**
       * Set SSL options
       * @param option The option or OR-ed options to apply.
       * @return The options applied.
       * @see https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_options.html
       */
      long setOptions( long option );

      /**
       * Trust all certificates in the specified directory.
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

      bool isAllowSANWildcards() const { return allow_san_wildcards_; }

    private:
      /**
       * Initialize the SSL library
       * @return nothing.
       */
      static void InitializeSSL();

      /**
       * Shutdown the SSL library
       * @return nothing.
       */
      static void ShutdownSSL();

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
