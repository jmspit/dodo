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
#include <common/exception.hpp>
#include <common/octetarray.hpp>
#include <openssl/err.h>

namespace dodo::common {

  /**
   * Interface to encrypt and decrypt OctetArray data to/from a secure string. Intended for smaller secrets such as
   * passwords, works for larger data volumes but with a time penalty, encryption/decryption of 10MiB on a
   * 3.4GHz Corei7 takes 0.8s/0.7s. Ideal for deployment configuration data.
   *
   * The format of the encrypted string is
   * @code
   * ENC[cipher:{cipher},data:{encrypted data},iv:{initialization vector},tag:{tag}]
   * @endcode
   *
   * for example
   *
   * @code
   * ENC[cipher:EVP_aes_256_gcm,data:dOxteDqw7POETW6RnDwWGVOUHkGf5OE7S1UY157ZEDx0Fv5vc9c=,iv:/GXteCh6FEt2IZbmgBurjA==,tag:WgKBIu/JgCZivZRTtb5A9Q==]
   * @endcode
   *
   * As the only external data required to decrypt is the key, and the other information required to decrypt included
   * in the encrypted string, it is robust against change and not coupled to specific programs or programming languages.
   * In fact, if the encryption uses DataCrypt::Cipher::Default, a simple decrypt/encrypt cycle will move to newer
   * ciphers without changing any code.
   *
   * The IV  and tag are not secrets. The iv needs to be random to prevent entropy loss with access to multiple
   * encrypted strings generated with the same key. The encrypt function will take care of generating unique IV's for
   * each call to encrypt. The tag is used to verify the decryption success, and is generated internally upon
   * encrypt.
   *
   * The available DataCrypt::Cipher choices are
   * | cipher | key size in bytes (octets) |
   * |--------|----------------------------|
   * |EVP_aes_128_gcm|16|
   * |EVP_aes_192_gcm|24|
   * |EVP_aes_256_gcm|32|
   *
   * If the provided key size is smaller, it is padded by repeating the specified key until filled. Use EVP_aes_256_gcm
   * with a strong 32 byte key for maximum safety.
   *
   * Only the data size in the encrypted string will depend on the size of the original data. The encrypted string
   * data will be around 1.4 times larger for an original data size of 38 bytes, and about 1.3 times for an original
   * data size of 240 bytes.
   *
   */
  class DataCrypt {
    public:

      /**
       * Cipher selection. GCM is the more secure block cipher, smaller key sizes ought to be faster
       * at the expense of work required to crack.
       */
      enum class Cipher {
        EVP_aes_128_gcm,  /**< https://www.openssl.org/docs/man1.0.2/man3/EVP_aes_128_gcm.html */
        EVP_aes_192_gcm,  /**< https://www.openssl.org/docs/man1.0.2/man3/EVP_aes_192_gcm.html */
        EVP_aes_256_gcm,  /**< https://www.openssl.org/docs/man1.0.2/man3/EVP_aes_256_gcm.html */
        Default = EVP_aes_256_gcm,
        Invalid
      };

      /**
       * Return the size of the key for the given Cipher in bits
       * @param cipher The Cipher to get the key bit size for.
       * @return the key size in bits;
       */
      static int keyOctets( Cipher cipher ) {
        switch ( cipher ) {
          case Cipher::EVP_aes_128_gcm : return 16;
          case Cipher::EVP_aes_192_gcm : return 24;
          case Cipher::EVP_aes_256_gcm : return 32;
          case Cipher::Invalid : return 0;
        }
        return 0;
      }

      /**
       * Return the size of the IV (initialization vector) for the given Cipher in bits
       * @param cipher The Cipher to get the iv bit size for.
       * @return the iv size in bits;
       */
      static int ivOctets( Cipher cipher ) {
        switch ( cipher ) {
          case Cipher::EVP_aes_128_gcm : return 16;
          case Cipher::EVP_aes_192_gcm : return 16;
          case Cipher::EVP_aes_256_gcm : return 16;
          case Cipher::Invalid : return 0;
        }
        return 0;
      }

      /**
       * Return the block size of the Cipher in octets.
       * @param cipher The Cipher
       * @return The size of the block in octets.
       */
      static int blockOctets( Cipher cipher ) {
        switch ( cipher ) {
          case Cipher::EVP_aes_128_gcm : return 16;
          case Cipher::EVP_aes_192_gcm : return 16;
          case Cipher::EVP_aes_256_gcm : return 16;
          case Cipher::Invalid : return 0;
        }
        return 0;
      }

      /**
       * Return the tag length of the Cipher in octets.
       * @param cipher The Cipher
       * @return The length of the tag in octets.
       */
      static int tagLength( Cipher cipher ) {
        switch ( cipher ) {
          case Cipher::EVP_aes_128_gcm : return 16;
          case Cipher::EVP_aes_192_gcm : return 16;
          case Cipher::EVP_aes_256_gcm : return 16;
          case Cipher::Invalid : return 0;
        }
        return 0;
      }

      /**
       * String representation of an Cipher instance.
       * @param cipher The Cipher to get the iv bit size for.
       * @return The string representation.
       */
      static std::string cipher2String( const Cipher& cipher ) {
        switch ( cipher ) {
          case Cipher::EVP_aes_128_gcm : return "EVP_aes_128_gcm";
          case Cipher::EVP_aes_192_gcm : return "EVP_aes_192_gcm";
          case Cipher::EVP_aes_256_gcm : return "EVP_aes_256_gcm";
          case Cipher::Invalid : return "Invalid cipher";
        }
        return "Invalid cipher";
      }

      /**
       * Convert a string representation to an Cipher.
       * @param s The string representation of an Cipher.
       * @return The Cipher.
       */
      static Cipher string2Cipher( const std::string &s ) {
        if ( s == cipher2String( Cipher::EVP_aes_128_gcm ) ) return Cipher::EVP_aes_128_gcm;
        else if ( s == cipher2String( Cipher::EVP_aes_192_gcm ) ) return Cipher::EVP_aes_192_gcm;
        else if ( s == cipher2String( Cipher::EVP_aes_256_gcm ) ) return Cipher::EVP_aes_256_gcm;
        else return Cipher::Invalid;
      }

      /**
       * Encrypt data with a key into a string (so the encrypted data will not contain a 0/zero).
       *
       * @param cipher The cipher to use
       * @param key The key to encrypt with.
       * @param src The source data to encrypt.
       * @param dst The encrypted string
       * @return void
       */
      static void encrypt( Cipher cipher,
                           const std::string &key,
                           const OctetArray& src,
                           std::string &dst );

      /**
       * Decrypt data with a key.
       * @param key The key to decrypt with.
       * @param src The source data to decrypt.
       * @param dest The OctetArray that will receive the decrypted data. The caller will become owner of the
       * pointer in OctetArray and responsible for cleaning it up with free().
       * @return 0 if decryption ok, 1 if failure due to the format not being recognized, 2 if key was incorrect
       * or the data corrupted.
       */
      static int decrypt( const std::string &key,
                          const std::string src,
                          OctetArray &dest );

    private:
      /**
       * Calculate the size of the encrypted data from the input size.
       * @param cipher The Cipher to calculate for.
       * @param octets The number of octets in the data to encrypt.
       * @return The size of the encrypted data.
       */
      static size_t cipherOctets( Cipher cipher, size_t octets ) {
        return octets + blockOctets( cipher ) - ( octets %  blockOctets( cipher ) );
      }

      /**
       * Decode an ENC[] string into its parts.
       * @param src The source encrypt string.
       * @param cipher The cipher string part.
       * @param data The data string part.
       * @param iv The iv string part.
       * @param tag The tag string part.
       * @return false if the decode failed / invalid src string.
       */
      static bool decode( const std::string &src,
                          std::string &cipher,
                          std::string &data,
                          std::string &iv,
                          std::string &tag );

      /**
       * Pad or trim a key to match the key size for the Cipher.
       * @param cipher The Cipher to apply.
       * @param key The key to adjust
       * @return The adjusted key.
       */
      static std::string paddedKey( Cipher cipher, const std::string key );

  };

}

#endif