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
 * @file datacrypt.cpp
 * Implements the dodo::common::DataCrypt class..
 */

#include "common/datacrypt.hpp"
#include "common/util.hpp"

#include <openssl/conf.h>
#include <openssl/err.h>

#include <iostream>

namespace dodo::common {

  void DataCrypt::encrypt( Algorithm algo,
                           const std::string &key,
                           const OctetArray& src,
                           std::string &dest ) {
    EVP_CIPHER_CTX *ctx = nullptr;

    if ( !( ctx = EVP_CIPHER_CTX_new() ) )
      throw_Exception( common::Puts() << "EVP_CIPHER_CTX_new : " << common::getSSLErrors( '\n' ) );


    OctetArray k = paddedKey( algo, key );
    OctetArray iv;
    OctetArray encrypted;
    iv.random( ivOctets( algo ) );
    encrypted.malloc( cipherOctets( algo, src.size ) );

    int rc = 0;
    switch ( algo ) {
      case Algorithm::EVP_aes_128_gcm :
        rc = EVP_EncryptInit_ex( ctx, EVP_aes_128_gcm(), nullptr, k.array, iv.array );
        break;
      case Algorithm::EVP_aes_192_gcm :
        rc = EVP_EncryptInit_ex( ctx, EVP_aes_192_gcm(), nullptr, k.array, iv.array );
        break;
      case Algorithm::EVP_aes_256_gcm :
        rc = EVP_EncryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, k.array, iv.array );
        break;
      case Algorithm::Invalid :
        throw_Exception( "cannot use Algorithm 'Invalid'" );
        break;
    }
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_EncryptInit_ex : " << common::getSSLErrors( '\n' ) );

    EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_AEAD_SET_IVLEN, ivOctets( algo ) * 8, nullptr );

    int enc_size = 0;
    int len = 0;
    rc = EVP_EncryptUpdate( ctx,
                            encrypted.array,
                            (int*)&len,
                            src.array,
                            (int)src.size );
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_EncryptUpdate : " << common::getSSLErrors( '\n' ) );
    enc_size += len;

    rc = EVP_EncryptFinal_ex( ctx,
                              encrypted.array + len,
                              &len);
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_EncryptFinal_ex : " << common::getSSLErrors( '\n' ) );
    enc_size += len;
    encrypted.size = enc_size;

    OctetArray tag;
    tag.malloc( tagLength( algo ) );
    if ( EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_GCM_GET_TAG, (int)tag.size, tag.array ) != 1 )
      throw_Exception( common::Puts() << "EVP_CIPHER_CTX_ctrl : " << common::getSSLErrors( '\n' ) );

    stringstream ss;
    ss << "ENC[cipher:" << algorithm2String( algo ) << ",";
    ss << "data:" << encrypted.encodeBase64();
    ss << ",iv:" << iv.encodeBase64();
    ss << ",tag:" << tag.encodeBase64();
    ss << "]";
    dest = ss.str();

    EVP_CIPHER_CTX_cleanup( ctx );
  }

  bool DataCrypt::decode( const std::string &src,
                          std::string &algo,
                          std::string &data,
                          std::string &iv,
                          std::string &tag ) {
    algo = "";
    data = "";
    iv   = "";
    tag  = "";
    if ( src.substr(0,4) != "ENC[" ) return false;
    if ( src.substr(src.length()-1,1) != "]" ) return false;
    std::vector<std::string> sections = split( src.substr(4,src.length()-5), ',' );
    if ( sections.size() != 4 ) return false;
    for ( auto s : sections ) {
      std::vector<std::string> tokens = split( s, ':' );
      if ( tokens.size() != 2 ) return false;
      if ( tokens[0] == "cipher" ) algo = tokens[1];
      else if ( tokens[0] == "data" ) data = tokens[1];
      else if ( tokens[0] == "iv" ) iv = tokens[1];
      else if ( tokens[0] == "tag" ) tag = tokens[1];
      else return false;
    }
    if ( algo.length() == 0 || data.length() == 0 || iv.length() == 0 || tag.length() == 0 ) return false;
    return true;
  }

  bool DataCrypt::decrypt( const std::string &key,
                           const std::string src,
                           OctetArray &dest ) {

    std::string salgo;
    std::string sdata;
    std::string siv;
    std::string stag;
    bool ok = decode( src, salgo, sdata, siv, stag );
    if ( !ok ) return false;
    Algorithm algo = string2Algorithm( salgo );
    OctetArray data;
    OctetArray iv;
    OctetArray tag;
    data.decodeBase64( sdata );
    iv.decodeBase64( siv );
    tag.decodeBase64( stag );
    //std::cout << "sdata " << sdata << std::endl;
    //std::cout << "siv " << siv << std::endl;
    //std::cout << "stag " << stag << std::endl;

    OctetArray k = paddedKey( algo, key );

    EVP_CIPHER_CTX *ctx = nullptr;

    if ( !( ctx = EVP_CIPHER_CTX_new() ) )
      throw_Exception( common::Puts() << "EVP_CIPHER_CTX_new : " << common::getSSLErrors( '\n' ) );

    int rc = 0;
    switch ( algo ) {
      case Algorithm::EVP_aes_128_gcm :
        rc = EVP_DecryptInit_ex( ctx, EVP_aes_128_gcm(), nullptr, k.array, iv.array );
        break;
      case Algorithm::EVP_aes_192_gcm :
        rc = EVP_DecryptInit_ex( ctx, EVP_aes_192_gcm(), nullptr, k.array, iv.array );
        break;
      case Algorithm::EVP_aes_256_gcm :
        rc = EVP_DecryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, k.array, iv.array );
        break;
      case Algorithm::Invalid :
        throw_Exception( common::Puts() << "invalid algorithm " << salgo );
        break;
    }
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_DecryptInit_ex : " << common::getSSLErrors( '\n' ) );

    EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_AEAD_SET_IVLEN, ivOctets( algo ) * 8, nullptr );

    EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_AEAD_SET_TAG, (int)tag.size, tag.array );

    OctetArray tmp;
    tmp.malloc( data.size );
    dest.malloc(0);

    int len = (int)data.size;

    rc = EVP_DecryptUpdate( ctx,
                            tmp.array,
                            (int*)&len,
                            data.array,
                            (int)data.size );
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_DecryptUpdate : " << common::getSSLErrors( '\n' ) );
    dest.append( tmp, len );

    len = (int)data.size;
    rc = EVP_DecryptFinal_ex( ctx,
                              tmp.array,
                              &len);
    //if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_DecryptFinal_ex : " << common::getSSLErrors( '\n' ) );
    if ( rc != 1 ) ok = false;
    dest.append( tmp, len );

    EVP_CIPHER_CTX_cleanup(ctx);

    return ok;
  }

  std::string DataCrypt::paddedKey( Algorithm algo, const std::string key ) {
    std::string temp = key;
    if ( temp.length() < (size_t)keyOctets( algo ) ) {
      temp += std::string( keyOctets( algo ) - temp.length(), '#' );
    }
    return temp.substr( 0, keyOctets( algo ) );
  }

}