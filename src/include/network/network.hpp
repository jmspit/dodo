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
 * @file network.hpp
 * Includes network headers and implements network::initLibrary() and network::closeLibrary().
 */

#include <network/address.hpp>
#include <network/basesocket.hpp>
#include <network/socket.hpp>
#include <network/x509cert.hpp>
#include <network/tlscontext.hpp>
#include <network/tlssocket.hpp>

namespace dodo::network {

  void initLibrary() {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_digests();
  }

  void closeLibrary() {
    ERR_free_strings();
    EVP_cleanup();
  }

}