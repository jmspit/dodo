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
 * @file httpmessage.hpp
 * Defines the dodo::network::protocol::http::HTTPMessage class.
 */

#ifndef dodo_network_protocol_http_httpmessage_hpp
#define dodo_network_protocol_http_httpmessage_hpp

#include <common/bytes.hpp>
#include <network/protocol/http/httpfragment.hpp>
#include <string>

namespace dodo {

  namespace network::protocol::http {

    /**
     * Generic HTTPMessage, parent to HTTPRequest and HTTPResponse.
     * @see https://tools.ietf.org/html/rfc2616#page-15
     * @remark This class is not thread safe, but does not use any global variables.
     */
    class HTTPMessage : public HTTPFragment {
      public:


        /**
         * CR character.
         */
        static const char charCR;

        /**
         * LF character.
         */
        static const char charLF;

        /**
         * Space character.
         */
        static const char charSP;

        /**
         * Horizontal tab character.
         */
        static const char charHT;

        /**
         * Default constructor.
         */
        HTTPMessage() : body_("") {};

        /**
         * Add a header to the HTTPMessage. Note that multtiple headers may be specified with the same key
         * but are supposed to be equavalent to a comma seperated list, so
         * @code
         * addHeader( "key", "1" );
         * addHeader( "key", "2" );
         * addHeader( "key", "3" );
         * @endcode
         * is equivalent to
         * @code
         * addHeader( "key", "1,2,3" );
         * @endcode
         * and this class transforms the former into the latter automatically.
         * @param key The header key.
         * @param value The header value.
         */
        void addHeader( const std::string &key, const std::string &value );

        /**
         * Replace the value of a header.
         * @param key the key to replace.
         * @param value the value to set.
         */
        void replaceHeader( const std::string &key, const std::string &value );

        /**
         * Return a reference to the headers map.
         * @return A reference to headers_;
         */
        const std::map<std::string,std::string>& getHeaders() const { return headers_; };

        /**
         * Return true when this header key exists in this HTTPMessage's headers.
         * @param header The header key to check for (lowercase).
         * @return True then the header exists in this HTTPMessage's headers.
         */
        bool hasHeader( const std::string &header ) const { return headers_.find(header) != headers_.end(); }

        /**
         * Clear all headers.
         */
        void clearHeaders() { headers_.clear(); };

        /**
         * Get a header value as a string into value.
         * @param key The header key.
         * @param value Receives the header value.
         * @return True if the key was found, false otherwise, in which case value is undefined.
         */
        bool getHeaderValue( const std::string &key, std::string &value ) const;

        /**
         * Get a header value as an unsigned long into value.
         * @param key The header key.
         * @param value Receives the header value.
         * @return True if the key was found and the value an integer, false otherwise, in which case value is
         * undefined.
         */
        bool getHeaderValue( const std::string &key, unsigned long &value ) const;

        /**
         * Return the HTTTMessage body.
         * @return The HTTPMessage body.
         */
        const common::Bytes& getBody() const { return body_; };

        /**
         * Set the body.
         * @param body The body to set.
         */
        void setBody( const std::string& body );

        /**
         * Send this HTTPMessage to the socket.
         * @param socket The socket to write to.
         * @return The commonSystemError that might accour.
         */
        virtual common::SystemError send( BaseSocket* socket ) = 0;


      protected:

       /**
         * Parse a header section and update headers_.
         * @param data The VirtualReadBuffer to read from.
         * @return The ParseResult.
         */
        ParseResult parseHeaders( VirtualReadBuffer &data );

        /**
         * Check is the char is a CTL character.
         * @param c The character to check.
         * @return true is c is a control character
         * @see https://tools.ietf.org/html/rfc2616#page-15
         */
        static bool isCTL( char c ) { return c <= 31 || c == 127; };

        /**
         * Call buffer.next() as long as buffer.get() is whitespace ( charSP or charHT).
         * @param data The VirtualReadBuffer to read from.
         * @return A SystemError, like data tata presenting a CR but no LF after it.
         */
        static common::SystemError eatSpace( VirtualReadBuffer& data );

        /**
         * Consume a CR LF sequence only when it is there. Unlike parseCRLF this function
         * will not fail when the CRLF is not found.
         * @param data The VirtualReadBuffer to read from.
         * @return peOk or peExpectCRLF or peSystemError. The latter can only be returned when a CR is encountered without
         * a LF after it.
         */
        ParseResult eatCRLF( VirtualReadBuffer& data );

        /**
         * Consume a CR LF sequence.
         * @param data The VirtualReadBuffer to read from.
         * @return The ParseResult
         */
        ParseResult parseCRLF( VirtualReadBuffer& data );

        /**
         * Parse a hexadecimal chunk size (a hex value followed by CR LF).
         * @param data The VirtualReadBuffer to read from.
         * @param value The value receiving the chunk size.
         * @return The ParseResult, either peOk, peExpectCRLF or peInvalidChunkHex.
         */
        ParseResult parseChunkHex( VirtualReadBuffer& data, unsigned long &value );

        /**
         * Parse a header field value.
         * @param data The VirtualReadBuffer to read from.
         * @param value The value receiving the field-value.
         * @return The ParseResult.
         */
        ParseResult parseFieldValue( VirtualReadBuffer &data, std::string &value );

        /**
         * Parse a token (such as a header field name).
         * @param buffer The HTTP data source.
         * @param token The value receiving the token.
         * @return The ParseResult.
         */
        ParseResult parseToken( VirtualReadBuffer& buffer, std::string &token );

        /**
         * Parse an unsigned integer.
         * @param data The VirtualReadBuffer to read from.
         * @param value The value receiving the unsigned int.
         * @return The ParseResult.
         */
        static ParseResult parseUInt( VirtualReadBuffer &data, unsigned int& value );

        /**
         * Parse a chunked body (transfer-encoding: chunked) and resturn it as a single string.
         * @param data The VirtualReadBuffer to read from.
         * @return The ParseError.
         */
        ParseResult parseChunkedBody(VirtualReadBuffer &data );

        /**
         * Parse a body and resturn it as a single string. Note that the content-length header must be present
         * and specify the correct body length.
         * @param data The VirtualReadBuffer to read from.
         * @return The ParseError.
         */
        virtual ParseResult parseBody( VirtualReadBuffer &data ) = 0;

        /**
         * Return true if the char is a separator.
         * @param c The char to check.
         * @return True if c is a separator, false otherwise.
         * @todo the order of checks can be optimized on character frequency.
         */
        static bool isSeparator( char c ) {
          return c == '(' || c == ')' || c == '<' || c == '>' || c == '@' ||
                 c == ',' || c == ';' || c == ':' || c == '\\' || c == '"' ||
                 c == '/' || c == '[' || c == ']' || c == '?' || c == '=' ||
                 c == '{' || c == '}' || c == charSP || c == charHT;
        }

        /**
         * Return true if the char is whitespace character (charSP or charHT)
         * @param c The char to check.
         * @return True if c is whitespace, false otherwise.
         * @todo the order of checks can be optimized on character frequency.
         */
        static bool isSP( char c ) {
          return c == charSP || c == charHT;
        }

        /**
         * The message headers.
         */
        std::map<std::string,std::string> headers_;

        /**
         * The message body (if any).
         */
        common::Bytes body_;

    };

  }

}

#endif