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
 * @file socketreadbuffer.hpp
 * Implements the dodo::network::SocketReadBuffer class.
 */

#include <network/socketreadbuffer.hpp>

#include <stdio.h>

namespace dodo {

  namespace network {

    SocketReadBuffer::SocketReadBuffer( BaseSocket* socket, size_t bufsize ) {
      socket_ = socket;
      if ( !socket_->getBlocking() ) throw_Exception( "SocketReadBuffer requires a blocking socket" );
      idx_ = 0;
      received_ = 0;
      bufsize_ = bufsize;
      buffer_ = new char[bufsize_];
    }

    SocketReadBuffer::~SocketReadBuffer() {
      delete buffer_;
    }

    common::SystemError SocketReadBuffer::underflow() {
      common::SystemError error = common::SystemError::ecOK;
      if ( idx_ >= received_ ) {
        underflows_++;
        error = socket_->receive( buffer_, bufsize_, received_ );
        if ( error == common::SystemError::ecOK ) idx_ = 0;
      }
      return error;
    }

    char SocketReadBuffer::get() const {
      return buffer_[idx_];
    }

    common::SystemError SocketReadBuffer::next() {
      idx_++;
      return underflow();
    }

    FileReadBuffer::FileReadBuffer( std::string filename, size_t bufsize ) : VirtualReadBuffer() {
      bufsize_ = bufsize;
      read_ = 0;
      idx_ = 0;
      file_ = fopen( filename.c_str(), "r");
      if ( !file_  ) throw_SystemException( common::Puts() << "file '" << filename << "' cannot be opened (", errno );
      buffer_ = new char [bufsize_];
      common::SystemError error = underflow();
      if ( error != common::SystemError::ecOK  ) throw_SystemException( common::Puts() << "file '" << filename << "' cannot be opened", errno );
    }

    FileReadBuffer::~FileReadBuffer() {
      fclose( file_ );
      delete[] buffer_;
    }

    common::SystemError FileReadBuffer::underflow() {
      common::SystemError error = common::SystemError::ecOK;
      if ( idx_ >= read_ ) {
        underflows_++;
        read_ = fread( buffer_, 1, bufsize_, file_ );
        idx_ = 0;
        if ( read_ == 0 ) error = common::SystemError::ecEAGAIN;
      }
      return error;
    }


    char FileReadBuffer::get() const {
      return buffer_[idx_];
    }

    common::SystemError FileReadBuffer::next() {
      idx_++;
      return underflow();
    }

  }

}
