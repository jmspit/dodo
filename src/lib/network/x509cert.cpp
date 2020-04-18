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
 * @file sslcert.cpp
 * Implements the dodo::network::SSLSocket class.
 */

#include "network/x509cert.hpp"
#include "common/exception.hpp"
#include "network/tlscontext.hpp"

#include <openssl/ssl.h>

#include <iostream>
#include <fstream>
#include <regex>


namespace dodo::network {

  //====================================================================================================================
  // X509Common
  //====================================================================================================================

  X509Common::X509Type X509Common::detectX509Type( const std::string file, std::string &tag ) {
    X509Common::X509Type result = X509Common::X509Type::Unknown;
    std::ifstream pem( file );
    std::string head = "";
    std::getline( pem, head );
    std::regex re("-----BEGIN (.*)-----");
    std::smatch match;
    tag = "";
    if  (std::regex_search( head, match, re ) && match.size() > 1 ) {
      tag = match.str(1);
    }
    if ( tag == "PRIVATE KEY" ) result = X509Common::X509Type::PrivateKey; else
    if ( tag == "ENCRYPTED PRIVATE KEY" ) result = X509Common::X509Type::PrivateKey; else
    if ( tag == "PUBLIC KEY" ) result = X509Common::X509Type::PublicKey; else
    if ( tag == "CERTIFICATE REQUEST" ) result = X509Common::X509Type::CertificateSigningRequest; else
    if ( tag == "CERTIFICATE" ) result = X509Common::X509Type::Certificate;

    return result;
  }

  std::string X509Common::bio2String( BIO* bio ) {
    char *data = NULL;
    long length = BIO_get_mem_data( bio, &data );
    BIO_get_mem_data( bio, &data );
    return std::string( data, length );
  }

  //====================================================================================================================
  // X509CertificateSigningRequest
  //====================================================================================================================

  X509_REQ* X509CertificateSigningRequest::loadPEM( const std::string file ) {
    BIO* pembio = BIO_new( BIO_s_file() );
    try {
      if ( pembio == nullptr ) throw_Exception( "X509Certificate::loadPEM BIO_new( BIO_s_file() ) failed " +
                                                TLSContext::getSSLErrors(';') );
      int rc = BIO_read_filename( pembio, file.c_str() );
      if ( !rc ) throw_Exception( "X509Certificate::loadPEM BIO_read_filename failed " +
                                  TLSContext::getSSLErrors(';') );
      X509_REQ* temp = PEM_read_bio_X509_REQ( pembio, nullptr, nullptr, nullptr );
      if ( temp == nullptr ) throw_Exception( "X509Certificate::loadPEM PEM_read_bio_X509_AUX failed " +
                                              TLSContext::getSSLErrors(';') );
      BIO_free( pembio );
      return temp;
    }
    catch ( common::Exception &e ) {
      BIO_free( pembio );
      throw;
    }
  }

  std::string X509CertificateSigningRequest::getSubject( const X509_REQ *cert ) {
    X509_NAME* name = X509_REQ_get_subject_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = bio2String( output_bio );
    BIO_free( output_bio );
    return tmp;
  }

  std::string X509CertificateSigningRequest::getFingerPrint( const X509_REQ *cert, const std::string hashname ) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    unsigned int hash_size;
    unsigned char hash[EVP_MAX_MD_SIZE];
    const EVP_MD * digest = EVP_get_digestbyname( hashname.c_str() );
    if ( digest == nullptr ) throw_Exception( "X509Certificate::fingerPrint EVP_get_digestbyname failed " +
                                              TLSContext::getSSLErrors(';') );
    int rc = X509_REQ_digest( cert, digest, hash, &hash_size);
    if ( !rc ) throw_Exception( "X509Certificate::fingerPrint X509_digest failed " +
                                TLSContext::getSSLErrors(';') );
    for( unsigned int pos = 0; pos < hash_size; pos++ ) {
      if ( pos ) ss << ":";
      ss << std::setw(2) << (unsigned int)hash[pos];
    }
    return ss.str();
  }

  //====================================================================================================================
  // X509Certificate
  //====================================================================================================================

  X509* X509Certificate::loadPEM( const std::string file ) {
    BIO* pembio = BIO_new( BIO_s_file() );
    try {
      if ( pembio == nullptr ) throw_Exception( "X509Certificate::loadPEM BIO_new( BIO_s_file() ) failed " +
                                                TLSContext::getSSLErrors(';') );
      int rc = BIO_read_filename( pembio, file.c_str() );
      if ( !rc ) throw_Exception( "X509Certificate::loadPEM BIO_read_filename failed " +
                                  TLSContext::getSSLErrors(';') );
      X509* temp = PEM_read_bio_X509_AUX( pembio, nullptr, nullptr, nullptr );
      if ( temp == nullptr ) throw_Exception( "X509Certificate::loadPEM PEM_read_bio_X509_AUX failed " +
                                              TLSContext::getSSLErrors(';') );
      BIO_free( pembio );
      return temp;
    }
    catch ( common::Exception &e ) {
      BIO_free( pembio );
      throw;
    }
  }

  std::string X509Certificate::getIssuer( const X509 *cert ) {
    X509_NAME* name = X509_get_issuer_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = bio2String( output_bio );
    BIO_free( output_bio );
    return tmp;
  }

  std::string X509Certificate::getSerial( const X509 *cert ) {
    const ASN1_INTEGER* val = X509_get0_serialNumber( cert );
    BIGNUM *bnser = ASN1_INTEGER_to_BN( val, nullptr );
    char *serialChar = BN_bn2hex(bnser);
    std::string tmp = serialChar;
    ::free( serialChar );
    BN_free(bnser);
    return tmp;
  }

  std::string X509Certificate::getSubject( const X509 *cert ) {
    X509_NAME* name = X509_get_subject_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = bio2String( output_bio );
    BIO_free( output_bio );
    return tmp;
  }

  std::string X509Certificate::getFingerPrint( const X509 *cert, const std::string hashname ) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase;
    unsigned int hash_size;
    unsigned char hash[EVP_MAX_MD_SIZE];
    const EVP_MD * digest = EVP_get_digestbyname( hashname.c_str() );
    if ( digest == nullptr ) throw_Exception( "X509Certificate::fingerPrint EVP_get_digestbyname failed " +
                                              TLSContext::getSSLErrors(';') );
    int rc = X509_digest( cert, digest, hash, &hash_size);
    if ( !rc ) throw_Exception( "X509Certificate::fingerPrint X509_digest failed " +
                                TLSContext::getSSLErrors(';') );
    for( unsigned int pos = 0; pos < hash_size; pos++ ) {
      if ( pos ) ss << ":";
      ss << std::setw(2) << (unsigned int)hash[pos];
    }
    return ss.str();
  }

}