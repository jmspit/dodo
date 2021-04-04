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
 * @file httpfragment.hpp
 * Defines the dodo::network::protocol::http::HTTPFragment class.
 */

#ifndef dodo_network_protocol_http_httpfragment_hpp
#define dodo_network_protocol_http_httpfragment_hpp

#include <network/socketreadbuffer.hpp>
#include <string>

namespace dodo {

  namespace network::protocol::http {



    /**
     * Generic HTTP fragment, either a complete (such as HTTPRequest) or incomplete
     * HTTP fragment (ssuch as a HTTPRequest::HTTPRequestLine) . Descendant classes implement
     * ParseResult parse( VirtualReadBuffer& buffer ) to extract a complete HTTP fragment.
     * Note that the buffer is expected to be blocking on reads, if the HTTPFragment
     * expects more data it will wait and in case of a read timeout, fail. If the parse
     * fails due to incomplete data within the timeout, the ParseResult return value is
     * `{ peSystemError, common::SystemError::ecEAGAIN }`.
     */
    class HTTPFragment {
      public:

        /**
         * Parse results returned by the parse functions.
         */
        enum ParseError : unsigned int {
          peOk = 0,                       /**< Ok */
          peIncomplete,                   /**< Not an error, but more data is expected to complete */
          peSystemError,                  /**< The parse failed due to system error SocketReadBuffer::getLastError() */
          peExpectCRLF,                   /**< A CR was not followed by an LF */
          peUnFinishedToken,              /**< A token is being parsed but is ending erroneously */
          peExpectingHeaderColon,         /**< A header field name was read, but no colon found after it. */
          peInvalidHeaderFieldValue,      /**< A header field value was being read, but it is invalid. */
          peInvalidHeaderListEnd,         /**< A header list is not ending properly. */
          peInvalidMethod,                /**< An invalid method was specified in the request line. */
          peInvalidHTTPVersion,           /**< An invalid HTTP version was specified in the request line. */
          peInvalidRequestLine,           /**< The request line is invalid. */
          peInvalidContentLength,         /**< The content length does not match the message body size. */
          peUnexpectedBody,               /**< A message body is present but should not be there. */
          peInvalidTransferEncoding,      /**< The specified transfer-encoding header is invalid. */
          peInvalidChunkHex,              /**< The hex chunk size is invalid. */
          peInvalidLastChunk,             /**< The last chunk does not have size 0. */
          peExpectingUnsignedInt,         /**< An unsigned int was expected. */
        };

        /**
         * Used to convey parsing succces. If parseError is peOk, the parse was succesfull.
         * If parseError == peSystemError, systemError will be set to something else then SystemError::ecOK.
         */
        struct ParseResult {

          /**
           * Default constructor sets to `{ parseError = peOk; systemError = common::SystemError::ecOK; }`
           */
          ParseResult() { parseError = peOk; systemError = common::SystemError::ecOK; }

          /**
           * Explicit constructor.
           * @param pe The ParseError to set.
           * @param se The SystemError to set.
           */
          ParseResult( ParseError pe,  common::SystemError se ) { parseError = pe; systemError = se; }

          /**
           * Test if `parseError == peOk && systemError == common::SystemError::ecOK`.
           * @return true when all is well.
           */
          bool ok() const { return parseError == peOk && systemError == common::SystemError::ecOK; }

          /**
           * Test if `systemError == common::SystemError::ecEAGAIN`
           * @return true if there was more data expected.
           */
          bool eof() const { return systemError == common::SystemError::ecEAGAIN; }

          /**
           * Set the systemError. Implictly sets parseError to peOK ig `se == common::SystemError::ecOK`
           * and to peSystemError if `se != common::SystemError::ecOK`
           * @param se The SystemError to set.
           */
          void setSystemError( common::SystemError se ) {
            systemError = se;
            if ( systemError != common::SystemError::ecOK ) parseError = peSystemError;
            else parseError = peOk;
          }

          /**
           * Return the systemError.
           * @return thesystemError part.
           */
          common::SystemError getSystemError() const { return systemError; }

          /**
           * Return the ParseResult as a human readable string.
           * @return The ParseResult as a string.
           */
          std::string asString() const { return common::Puts() << "pe:" << getParseResultAsString(parseError) << "/se:" << systemError.asString(); }

          private:
            /** The parse eror. */
            ParseError parseError;
            /** The common::SystemError is parseError == peSystemError, undefined otherwise.*/
            common::SystemError systemError;
        };


        /**
         * Read a complete HTTPFragment from a VirtualReadBuffer.
         * @param buffer the VirtualReadBuffer to get characters from.
         * @return The ParseError.
         */
        virtual ParseResult parse( VirtualReadBuffer& buffer ) = 0;

        /**
         * Convert the HTTPFragment in a HTTP protocol string.
         */
        virtual std::string asString() const = 0;

        /**
         * Return the string description of a ParseError.
         * @param error The ParseError to describe.
         * @return The ParseError description.
         */
        static std::string getParseResultAsString( ParseError error );

    };

  }

}

#endif