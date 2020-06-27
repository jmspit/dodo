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
 * @file datacrypt.hpp
 * Defines the dodo::common::DataCrypt class.
 */

#ifndef common_datacrypt_hpp
#define common_datacrypt_hpp

#include <string>
#include "common/exception.hpp"
#include "common/octetarray.hpp"
#include <openssl/err.h>

namespace dodo::common {

  /**
   * Interface to encrypt and decrypt OctetArray data to/from a secure string.
   * The format of the encrypted string is
   * @code
   * ENC[{algorithm},data:{encrypted data},iv:{initialization vector}]
   * @endcode
   * The IV is not a secret, but it needs to be random (differ when applied to the same key in multiple instances)
   * or information can be inferred from encrypted instances sharing the same key+IV. The encrypt function
   * will take care of generating unique IV's for each call to encrypt.
   */
  class DataCrypt {
    public:

      /**
       * Algorithm selection. GCM is the more secure block cipher, smaller key sizes ought to be faster
       * at the expense of work required to crack.
       */
      enum class Algorithm {
        EVP_aes_128_gcm,  /**< https://www.openssl.org/docs/man1.0.2/man3/EVP_aes_128_gcm.html */
        EVP_aes_192_gcm,  /**< https://www.openssl.org/docs/man1.0.2/man3/EVP_aes_192_gcm.html */
        EVP_aes_256_gcm,  /**< https://www.openssl.org/docs/man1.0.2/man3/EVP_aes_256_gcm.html */
        Invalid
      };

      /**
       * Return the size of the key for the given Algorithm in bits
       * @param algo The Algorithm to get the key bit size for.
       * @return the key size in bits;
       */
      static int keyOctets( Algorithm algo ) {
        switch ( algo ) {
          case Algorithm::EVP_aes_128_gcm : return 16;
          case Algorithm::EVP_aes_192_gcm : return 24;
          case Algorithm::EVP_aes_256_gcm : return 32;
          case Algorithm::Invalid : return 0;
        }
        return 0;
      }

      /**
       * Return the size of the IV (initialization vector) for the given Algorithm in bits
       * @param algo The Algorithm to get the iv bit size for.
       * @return the iv size in bits;
       */
      static int ivOctets( Algorithm algo ) {
        switch ( algo ) {
          case Algorithm::EVP_aes_128_gcm : return 16;
          case Algorithm::EVP_aes_192_gcm : return 16;
          case Algorithm::EVP_aes_256_gcm : return 16;
          case Algorithm::Invalid : return 0;
        }
        return 0;
      }

      /**
       * Return the block size of the Algorithm in octets.
       * @param algo The Algorithm
       * @return The size of the block in octets.
       */
      static int blockOctets( Algorithm algo ) {
        switch ( algo ) {
          case Algorithm::EVP_aes_128_gcm : return 16;
          case Algorithm::EVP_aes_192_gcm : return 16;
          case Algorithm::EVP_aes_256_gcm : return 16;
          case Algorithm::Invalid : return 0;
        }
        return 0;
      }

      /**
       * Return the tag length of the Algorithm in octets.
       * @param algo The Algorithm
       * @return The length of the tag in octets.
       */
      static int tagLength( Algorithm algo ) {
        switch ( algo ) {
          case Algorithm::EVP_aes_128_gcm : return 16;
          case Algorithm::EVP_aes_192_gcm : return 16;
          case Algorithm::EVP_aes_256_gcm : return 16;
          case Algorithm::Invalid : return 0;
        }
        return 0;
      }

      /**
       * String representation of an Algorithm instance.
       * @param algo The Algorithm to get the iv bit size for.
       * @return The string representation.
       */
      static std::string algorithm2String( const Algorithm& algo ) {
        switch ( algo ) {
          case Algorithm::EVP_aes_128_gcm : return "EVP_aes_128_gcm";
          case Algorithm::EVP_aes_192_gcm : return "EVP_aes_192_gcm";
          case Algorithm::EVP_aes_256_gcm : return "EVP_aes_256_gcm";
          case Algorithm::Invalid : return "Invalid algorithm";
        }
        return "Invalid algorithm";
      }

      /**
       * Convert a string representation to an Algorithm.
       * @param s The string representation of an Algorithm.
       * @return The Algorithm.
       */
      static Algorithm string2Algorithm( const std::string &s ) {
        if ( s == algorithm2String( Algorithm::EVP_aes_128_gcm ) ) return Algorithm::EVP_aes_128_gcm;
        else if ( s == algorithm2String( Algorithm::EVP_aes_192_gcm ) ) return Algorithm::EVP_aes_192_gcm;
        else if ( s == algorithm2String( Algorithm::EVP_aes_256_gcm ) ) return Algorithm::EVP_aes_256_gcm;
        else return Algorithm::Invalid;
      }

      /**
       * Encrypt data with a key into a string (so the encrypted data will not contain a 0/zero).
       *
       * @param algo The algorithm to use
       * @param key The key to encrypt with.
       * @param src The source data to encrypt.
       * @param dst The encrypted string
       * @return void
       */
      static void encrypt( Algorithm algo,
                           const std::string &key,
                           const OctetArray& src,
                           std::string &dst );

      /**
       * Decrypt data with a key.
       * @param key The key to decrypt with.
       * @param src The source data to decrypt.
       * @param dest The OctetArray that will receive the decrypted data. The caller will become owner of the
       * pointer in OctetArray and responsible for cleaning it up with free().
       * @return True if decryption succeeded.
       */
      static bool decrypt( const std::string &key,
                           const std::string src,
                           OctetArray &dest );

    private:
      /**
       * Calculate the size of the encrypted data from the input size.
       * @param algo The Algorithm to calculate for.
       * @param octets The number of octets in the data to encrypt.
       * @return The size of the encrypted data.
       */
      static size_t cipherOctets( Algorithm algo, size_t octets ) {
        return octets + blockOctets( algo ) - ( octets %  blockOctets( algo ) );
      }

      /**
       * Decode an ENC[] string into its parts.
       * @param src The source encrypt string.
       * @param algo The algorithm string part.
       * @param data The data string part.
       * @param iv The iv string part.
       * @param tag The tag string part.
       * @return false if the decode failed / invalid src string.
       */
      static bool decode( const std::string &src,
                          std::string &algo,
                          std::string &data,
                          std::string &iv,
                          std::string &tag );

      /**
       * Pad or trim a key to match the key size for the Algorithm.
       * @param algo The Algorithm to apply.
       * @param key The key to adjust
       * @return The adjusted key.
       */
      static std::string paddedKey( Algorithm algo, const std::string key );

  };

}

#endif