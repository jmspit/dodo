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
 * Defines the dodo::network::SocketReadBuffer class.
 */

#ifndef dodo_network_socketreadbuffer_hpp
#define dodo_network_socketreadbuffer_hpp

#include <network/basesocket.hpp>

namespace dodo {

  using namespace common;

  namespace network {

    /**
     * Interface to read individual bytes whilst the implementation
     * can read from an actual source (such as a network buffer)
     * in larger chunks, reducing the number of required system calls. This is covenient
     * when implementing protocols that need be char (byte) scanned to parse.
     *
     * Note that this interface requires a source that blocks on read. A call to next()
     * will advance the current byte, but this in turn will call underflow, which in turn might do
     * a read if it reached the end of available bytes.
     *
     * @see SocketReadBuffer and FileReadBuffer - both concrete implementations.
     */
    class VirtualReadBuffer {
      public:

        VirtualReadBuffer() : buffer_(NULL), bufsize_(0) {};

        /**
         * Get the current char from VirtualReadBuffer.
         * @return The current character read.
         * @see next()
         */
        virtual char get() const = 0;

        /**
         * Move to the next char from the VirtualReadBuffer. Note that call will return a SystemError::ecEAGAIN
         * if there is no more data (received within the prevailing timeout).
         * @return SystemError::ecOK and c is valid, or an SystemError.
         */
        virtual SystemError next() = 0 ;

      protected:

        /**
         * If the buffer_ underflows, the buffer_ must be reset refilledwith new data from the source.
         * @return Systemerror::ecOK on succes.
         */
        virtual SystemError underflow() = 0;

        /**
         * The buffer.
         * @see bufsize_
         */
        char*  buffer_;

        /**
         * The size of buffer_.
         */
        ssize_t bufsize_;
    };

    /**
     * SocketReadBuffer is a VirtualReadBuffer that reads from the BaseSocket
     * in chunks internally. Note that the BaseSocket should be in blocking mode.
     */
    class SocketReadBuffer : public VirtualReadBuffer {
      public:

        /**
         * Construct a SocketReadBuffer. If BaseSocket::getBlocking() is false, an exception is thrown.
         * @param socket The blocking BaseSocket to read from.
         * @param bufsize The buffer size to use
         */
        explicit SocketReadBuffer( BaseSocket* socket, size_t bufsize = 8192 );

        /**
         * Destruct the SocketReadBuffer, de-allocate the internal buffer.
         * Any bytes buffered will be lost.
         */
        virtual ~SocketReadBuffer();

        virtual char get() const;

        virtual SystemError next();

      protected:

        /**
         * If the buffer_ is fully read. eset and read new data from the socket.
         * @return Systemerror::ecOK on succes.
         */
        virtual SystemError underflow();

        /**
         * The associated network::BaseSocket*.
         */
        BaseSocket* socket_;

        /**
         * The current index of get(char &c) in buffer_.
         */
        ssize_t idx_;

        /**
         * The number of chars received in the last underflow().
         */
        ssize_t received_;

    };

    /**
     * File-based VirtualReadBuffer, conventient for testing purposes as
     * parsers can be tested against file instead of network sources.
     */
    class FileReadBuffer : public VirtualReadBuffer {
      public:

        /**
         * Construct a FileReadBuffer.
         * @param filename The file to read.
         * @param bufsize The buffer size to use
         */
        explicit FileReadBuffer( std::string filename, size_t bufsize = 8192 );

        virtual ~FileReadBuffer();

        virtual char get() const;

        virtual SystemError next();

      protected:

        virtual SystemError underflow();

        /** The file handle. */
        FILE *file_;

        /** The index into the buffer. */
        size_t idx_;

        /**
         * The number of chars read in buffer_
         */
        size_t read_;

    };

    /**
     * String based VirtualReadBuffer, conventient for testing purposes as
     * parsers can be tested against strings instead of network sources.
     */
    class StringReadBuffer : public VirtualReadBuffer {
      public:

        /**
         * Construct a FileReadBuffer.
         * @param filename The file to read.
         * @param bufsize The buffer size to use
         */
        explicit StringReadBuffer( const std::string &data ) { data_ = data; idx_ = 0; }

        virtual ~StringReadBuffer() {};

        virtual char get() const { return data_.at(idx_); }

        virtual SystemError next() { if ( idx_ >= data_.length() -1  ) return common::SystemError::ecEAGAIN; else { idx_++; return common::SystemError::ecOK; } };

      protected:

        virtual SystemError underflow() { return common::SystemError::ecOK; };

        std::string data_;

        size_t idx_;

    };

  }

}

#endif
