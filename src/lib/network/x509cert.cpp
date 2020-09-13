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
 * @file x509cert.cpp
 * Implements the dodo::network::SSLSocket class.
 */

#include <fstream>
#include <iostream>
#include <openssl/ssl.h>
#include <regex>

#include "common/exception.hpp"
#include "common/util.hpp"
#include "network/address.hpp"
#include "network/tlscontext.hpp"
#include "network/x509cert.hpp"


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

  X509Common::Identity X509Common::parseIdentity( const std::string src ) {
    Identity identity;
    //std::cout << "! " << src << endl;
    std::vector<std::string> items = common::escapedSplit( src, {'\\'}, ',' );
    for ( auto item : items ) {
      std::vector<std::string> kvpair = common::split( item, '=' );
      //std::cout << "!! " << kvpair[0] << endl;
      if ( kvpair.size() == 2 ) {
        if ( kvpair[0] == "C" ) identity.countryCode = kvpair[1];
        else if ( kvpair[0] == "ST" ) identity.state = kvpair[1];
        else if ( kvpair[0] == "L" ) identity.locality = kvpair[1];
        else if ( kvpair[0] == "O" ) identity.organization = kvpair[1];
        else if ( kvpair[0] == "OU" ) identity.organizationUnit = kvpair[1];
        else if ( kvpair[0] == "CN" ) identity.commonName = kvpair[1];
        else if ( kvpair[0] == "emailAddress" ) identity.email = kvpair[1];
        else if ( kvpair[0] == "businessCategory" ) identity.businessCategory = kvpair[1];
        else if ( kvpair[0] == "jurisdictionC" ) identity.jurisdictionC = kvpair[1];
        else if ( kvpair[0] == "jurisdictionST" ) identity.jurisdictionST = kvpair[1];
        else if ( kvpair[0] == "serialNumber" ) identity.serialNumber = kvpair[1];
        else if ( kvpair[0] == "street" ) identity.street = kvpair[1];
        else if ( kvpair[0] == "postalCode" ) identity.postalCode = kvpair[1];
        else identity.other[ kvpair[0] ] = kvpair[1];
      }
    }
    return identity;
  }

  //====================================================================================================================
  // X509CertificateSigningRequest
  //====================================================================================================================

  X509_REQ* X509CertificateSigningRequest::loadPEM( const std::string file ) {
    BIO* pembio = BIO_new( BIO_s_file() );
    try {
      if ( pembio == nullptr ) throw_Exception( "BIO_new( BIO_s_file() ) failed " +
                                                common::getSSLErrors(';') );
      int rc = BIO_read_filename( pembio, file.c_str() );
      if ( !rc ) throw_Exception( "BIO_read_filename failed " +
                                  common::getSSLErrors(';') );
      X509_REQ* temp = PEM_read_bio_X509_REQ( pembio, nullptr, nullptr, nullptr );
      if ( temp == nullptr ) throw_Exception( "PEM_read_bio_X509_AUX failed " +
                                              common::getSSLErrors(';') );
      BIO_free( pembio );
      return temp;
    }
    catch ( common::Exception &e ) {
      BIO_free( pembio );
      throw;
    }
  }

  X509Common::Identity X509CertificateSigningRequest::getSubject( const X509_REQ *cert ) {
    X509_NAME* name = X509_REQ_get_subject_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = dodo::common::bio2String( output_bio );
    BIO_free( output_bio );
    return parseIdentity( tmp );
  }

  std::string X509CertificateSigningRequest::getFingerPrint( const X509_REQ *cert, const std::string hashname ) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    unsigned int hash_size;
    unsigned char hash[EVP_MAX_MD_SIZE];
    const EVP_MD * digest = EVP_get_digestbyname( hashname.c_str() );
    if ( digest == nullptr ) throw_Exception( "EVP_get_digestbyname failed " +
                                              common::getSSLErrors(';') );
    int rc = X509_REQ_digest( cert, digest, hash, &hash_size);
    if ( !rc ) throw_Exception( "X509_digest failed " +
                                common::getSSLErrors(';') );
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
      if ( pembio == nullptr ) throw_Exception( "BIO_new( BIO_s_file() ) failed " +
                                                common::getSSLErrors(';') );
      int rc = BIO_read_filename( pembio, file.c_str() );
      if ( !rc ) throw_Exception( "BIO_read_filename failed " +
                                  common::getSSLErrors(';') );
      X509* temp = PEM_read_bio_X509_AUX( pembio, nullptr, nullptr, nullptr );
      if ( temp == nullptr ) throw_Exception( "PEM_read_bio_X509_AUX failed " +
                                              common::getSSLErrors(';') );
      BIO_free( pembio );
      return temp;
    }
    catch ( common::Exception &e ) {
      BIO_free( pembio );
      throw;
    }
  }

  X509Common::Identity X509Certificate::getIssuer( const X509 *cert ) {
    X509_NAME* name = X509_get_issuer_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = dodo::common::bio2String( output_bio );
    BIO_free( output_bio );
    return parseIdentity(tmp);
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

  X509Common::Identity X509Certificate::getSubject( const X509 *cert ) {
    X509_NAME* name = X509_get_subject_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = dodo::common::bio2String( output_bio );
    BIO_free( output_bio );
    //std::cout << tmp << std::endl;
    return parseIdentity( tmp );
  }

  std::list<X509Certificate::SAN> X509Certificate::getSubjectAltNames( const X509* cert ) {
    std::list<X509Certificate::SAN> result;
    GENERAL_NAMES* subjectaltnames = (GENERAL_NAMES*)X509_get_ext_d2i( cert, NID_subject_alt_name, NULL, NULL);
    if ( !subjectaltnames ) return result;
    int altnamecount = sk_GENERAL_NAME_num(subjectaltnames);
    for ( int i = 0; i < altnamecount; i++ ) {
      GENERAL_NAME* generalname = sk_GENERAL_NAME_value( subjectaltnames, i) ;
      if ( generalname ) {
        if ( generalname->type == GEN_URI ||
             generalname->type == GEN_DNS ||
             generalname->type == GEN_EMAIL ) {
          std::string san = std::string(reinterpret_cast<char*>(generalname->d.ia5->data));
          result.push_back( { static_cast<X509Common::SANType>(generalname->type), san } );
        } else if ( generalname->type == GEN_IPADD)  {
          unsigned char *data = generalname->d.ip->data;
          if ( generalname->d.ip->length == 4 ) {
            std::stringstream ip;
            ip << (int)data[0] << '.' << (int)data[1] << '.' << (int)data[2] << '.' << (int)data[3];
            result.push_back( { static_cast<X509Common::SANType>(generalname->type), ip.str() } );
          } else {
            const unsigned char *data = ASN1_STRING_get0_data( generalname->d.iPAddress );
            int datalen = ASN1_STRING_length( generalname->d.ia5 );
            const unsigned char *p = data;
            std::stringstream ip;
            for ( int i = 0; i < datalen/2 ; i++ ) {
              if ( i > 0 ) ip << ":";
              ip << std::hex << (int)(p[0] << 8 | p[1]);
              p+=2;
            }
            result.push_back( { static_cast<X509Common::SANType>(generalname->type), ip.str() } );
          }
        }
      }
    }
    if ( subjectaltnames ) sk_GENERAL_NAME_pop_free( subjectaltnames, GENERAL_NAME_free );
    return result;
  }

  std::string X509Certificate::getFingerPrint( const X509 *cert, const std::string hashname ) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase;
    unsigned int hash_size;
    unsigned char hash[EVP_MAX_MD_SIZE];
    const EVP_MD * digest = EVP_get_digestbyname( hashname.c_str() );
    if ( digest == nullptr ) throw_Exception( "EVP_get_digestbyname failed " +
                                              common::getSSLErrors(';') );
    int rc = X509_digest( cert, digest, hash, &hash_size);
    if ( !rc ) throw_Exception( "X509_digest failed " +
                                common::getSSLErrors(';') );
    for( unsigned int pos = 0; pos < hash_size; pos++ ) {
      if ( pos ) ss << ":";
      ss << std::setw(2) << (unsigned int)hash[pos];
    }
    return ss.str();
  }

  bool X509Certificate::verifyName( const std::string peer, const std::string san, bool allowwildcard ) {
    if ( peer.length() < san.length() ) return false;
    auto itr_peer = peer.rbegin();
    auto itr_san = san.rbegin();
    while ( itr_peer != peer.rend() && itr_san != san.rend() ) {
      if ( *itr_peer != *itr_san ) {
        if ( allowwildcard && *itr_san == '*' ) {
          // verify there are no dots to the left
          while ( itr_peer != peer.rend() ) {
            if ( *itr_peer == '.' ) return false;
            itr_peer++;
          }
          return true;
        } else return false;
      }
      itr_peer++;
      itr_san++;
    }
    return peer.length() == san.length();
  }

  bool X509Certificate::verifyIP( const std::string peer, const std::string san ) {
    network::Address addr_peer = peer;
    network::Address addr_san = san;
    if ( addr_peer.isValid() && addr_san.isValid() && addr_peer == addr_san )
      return true;
    else
      return false;
  }

  bool X509Certificate::verifySAN( const X509 *cert, const SAN &san ) {
    bool verified = false;
    auto cert_sans = getSubjectAltNames( cert );
    X509Common::Identity subject = getSubject( cert );
    cert_sans.push_front( { san.san_type, subject.commonName } );
    switch ( san.san_type ) {
      case SANType::stDNS:
      case SANType::stURI:
      case SANType::stEMAIL:
        for ( auto s : cert_sans ) {
          if ( s.san_type == san.san_type && verifyName( san.san_name, s.san_name, true ) ) {
            verified = true;
            break;
          }
        }
        break;
      case SANType::stIP:
        for ( auto s : cert_sans ) {
          if ( s.san_type == san.san_type && verifyIP( san.san_name, s.san_name ) ) {
            verified = true;
            break;
          }
        }
        break;
    }
    return verified;
  }

}