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


namespace dodo::network {

  std::string X509Certificate::getSubject( const X509 *cert ) {
    X509_NAME* name = X509_get_subject_name( cert );
    BIO* output_bio = BIO_new( BIO_s_mem() );
    X509_NAME_print_ex( output_bio, name, 0, XN_FLAG_RFC2253 );
    std::string tmp = bio2String( output_bio );
    BIO_free( output_bio );
    return tmp;
  }

  std::string X509Certificate::bio2String( BIO* bio ) {
    char *data = NULL;
    //long length = BIO_get_mem_data( bio, &data );
    BIO_get_mem_data( bio, &data );
    return std::string( data );
  }

}