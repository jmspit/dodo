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

  void DataCrypt::encrypt( Cipher cipher,
                           const std::string &key,
                           const OctetArray& src,
                           std::string &dest ) {
    EVP_CIPHER_CTX *ctx = nullptr;

    if ( !( ctx = EVP_CIPHER_CTX_new() ) )
      throw_Exception( common::Puts() << "EVP_CIPHER_CTX_new : " << common::getSSLErrors( '\n' ) );


    OctetArray k = paddedKey( cipher, key );
    OctetArray iv;
    OctetArray encrypted;
    iv.random( ivOctets( cipher ) );
    encrypted.reserve( cipherOctets( cipher, src.size ) );

    int rc = 0;
    switch ( cipher ) {
      case Cipher::EVP_aes_128_gcm :
        rc = EVP_EncryptInit_ex( ctx, EVP_aes_128_gcm(), nullptr, k.array, iv.array );
        break;
      case Cipher::EVP_aes_192_gcm :
        rc = EVP_EncryptInit_ex( ctx, EVP_aes_192_gcm(), nullptr, k.array, iv.array );
        break;
      case Cipher::EVP_aes_256_gcm :
        rc = EVP_EncryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, k.array, iv.array );
        break;
      case Cipher::Invalid :
        throw_Exception( "cannot use Cipher 'Invalid'" );
        break;
    }
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_EncryptInit_ex : " << common::getSSLErrors( '\n' ) );

    EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_AEAD_SET_IVLEN, ivOctets( cipher ) * 8, nullptr );

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
    tag.reserve( tagLength( cipher ) );
    if ( EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_GCM_GET_TAG, (int)tag.size, tag.array ) != 1 )
      throw_Exception( common::Puts() << "EVP_CIPHER_CTX_ctrl : " << common::getSSLErrors( '\n' ) );

    stringstream ss;
    ss << "ENC[cipher:" << cipher2String( cipher ) << ",";
    ss << "data:" << encrypted.encodeBase64();
    ss << ",iv:" << iv.encodeBase64();
    ss << ",tag:" << tag.encodeBase64();
    ss << "]";
    dest = ss.str();

    EVP_CIPHER_CTX_cleanup( ctx );
  }

  bool DataCrypt::decode( const std::string &src,
                          std::string &cipher,
                          std::string &data,
                          std::string &iv,
                          std::string &tag ) {
    cipher = "";
    data = "";
    iv   = "";
    tag  = "";
    if ( src.substr(0,4) != "ENC[" ) return false;
    size_t close_pos = src.length() -1;
    while ( std::isspace( src[close_pos] ) && close_pos > 0 ) close_pos--;
    if ( src.substr(close_pos,1) != "]" ) return false;
    std::vector<std::string> sections = split( src.substr(4,close_pos-4), ',' );
    if ( sections.size() != 4 ) return false;
    for ( auto s : sections ) {
      std::vector<std::string> tokens = split( s, ':' );
      if ( tokens.size() != 2 ) return false;
      if ( tokens[0] == "cipher" ) cipher = tokens[1];
      else if ( tokens[0] == "data" ) data = tokens[1];
      else if ( tokens[0] == "iv" ) iv = tokens[1];
      else if ( tokens[0] == "tag" ) tag = tokens[1];
      else return false;
    }
    if ( cipher.length() == 0 || data.length() == 0 || iv.length() == 0 || tag.length() == 0 ) return false;
    return true;
  }

  int DataCrypt::decrypt( const std::string &key,
                          const std::string src,
                          OctetArray &dest ) {

    std::string scipher;
    std::string sdata;
    std::string siv;
    std::string stag;
    bool ok = decode( src, scipher, sdata, siv, stag );
    if ( !ok ) return 1;
    Cipher cipher = string2Cipher( scipher );
    OctetArray data;
    OctetArray iv;
    OctetArray tag;
    data.decodeBase64( sdata );
    iv.decodeBase64( siv );
    tag.decodeBase64( stag );

    OctetArray k = paddedKey( cipher, key );

    EVP_CIPHER_CTX *ctx = nullptr;

    if ( !( ctx = EVP_CIPHER_CTX_new() ) )
      throw_Exception( common::Puts() << "EVP_CIPHER_CTX_new : " << common::getSSLErrors( '\n' ) );

    int rc = 0;
    switch ( cipher ) {
      case Cipher::EVP_aes_128_gcm :
        rc = EVP_DecryptInit_ex( ctx, EVP_aes_128_gcm(), nullptr, k.array, iv.array );
        break;
      case Cipher::EVP_aes_192_gcm :
        rc = EVP_DecryptInit_ex( ctx, EVP_aes_192_gcm(), nullptr, k.array, iv.array );
        break;
      case Cipher::EVP_aes_256_gcm :
        rc = EVP_DecryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, k.array, iv.array );
        break;
      case Cipher::Invalid :
        throw_Exception( common::Puts() << "invalid cipher " << scipher );
        break;
    }
    if ( rc != 1 ) throw_Exception( common::Puts() << "EVP_DecryptInit_ex : " << common::getSSLErrors( '\n' ) );

    EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_AEAD_SET_IVLEN, ivOctets( cipher ) * 8, nullptr );

    EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_AEAD_SET_TAG, (int)tag.size, tag.array );

    OctetArray tmp;
    tmp.reserve( data.size );
    dest.reserve(0);

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
    if ( rc != 1 ) ok = 2;
    dest.append( tmp, len );

    EVP_CIPHER_CTX_cleanup(ctx);

    return 0;
  }

  std::string DataCrypt::paddedKey( Cipher cipher, const std::string key ) {
    std::string temp = key;
    if ( temp.length() < (size_t)keyOctets( cipher ) ) {
      temp += std::string( keyOctets( cipher ) - temp.length(), '#' );
    }
    return temp.substr( 0, keyOctets( cipher ) );
  }

}