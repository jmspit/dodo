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
 * @file httprequest.hpp
 * Defines the dodo::network::protocol::http::HTTPRequest class.
 */

#ifndef dodo_network_protocol_http_httprequest_hpp
#define dodo_network_protocol_http_httprequest_hpp

#include <network/protocol/http/httpmessage.hpp>
#include <network/protocol/http/httpversion.hpp>
#include <string>

namespace dodo {

  namespace network::protocol::http {

    /**
     * HTTPRequest class represents a HTTP request. A HTTPRequest contains
     * - A HTTPRequestLine
     * - An optional body of type common::Bytes.
     *
     * @include examples/http-client/http-client.cpp
     */
    class HTTPRequest : public HTTPMessage {
      public:

        /**
         * The HTTP request method.
         */
        enum Method {
          meGET,
          meHEAD,
          mePOST,
          mePUT,
          mePATCH,
          meDELETE,
          meCONNECT,
          meOPTIONS,
          meTRACE,
          meINVALID, /**< The parser was presented an invalid method. */
        };

        /**
         * HTTPRequestLine class. Contains
         * - A request method HTTPRequest::Method.
         * - A request 'uri' (it is not an uri buth rather a path that could be an URI).
         * - A HTTP version
         */
        class HTTPRequestLine : public HTTPFragment {
          public:

            /**
             * Construct default HTTPRequestLine.
             */
            HTTPRequestLine() : method_(HTTPRequest::meGET),
                                request_uri_("\"*\""),
                                http_version_()
                                {};

            /**
             * Convert the HTTPRequestLine to a HTTP string.
             * @return The HTTP string.
             */
            virtual std::string asString() const;

            /**
             * Parses a HTTPMessage
             * @param data The VirtualReadBuffer to read from.
             * @return The ParseResult.
             */
            virtual ParseResult parse( VirtualReadBuffer &data );

            /**
             * Get the HTTPRequest::Method.
             * @return the HTTPRequest::Method.
             */
            HTTPRequest::Method getMethod()  const { return method_; };

            /**
             * Set the HTTPRequest::Method.
             * @param method The HTTPRequest::Method.
             */
            void setMethod( HTTPRequest::Method method ) { method_ = method; };

            /**
             * Get the request uri.
             * @return the request uri.
             */
            std::string getRequestURI() const { return request_uri_; };

            /**
             * Set the request uri.
             * @param uri The request uri.
             */
            void setRequestURI( const std::string &uri ) { request_uri_ = uri; };

            /**
             * Get the HTTP version.
             * @return The HTTP version.
             */
            HTTPVersion getHTTPVersion() const { return http_version_; };

            /**
             * Set the HTTP version.
             * @param version The HTTP version.
             */
            void setHTTPVersion( const HTTPVersion &version ) { http_version_ = version; };

          protected:

            /**
             * The request method.
             */
            HTTPRequest::Method method_;

            /**
             * The request 'uri'.
             */
            std::string request_uri_;

            /**
             * The HTTP version.
             */
            HTTPVersion http_version_;
        };

        HTTPRequest() : request_line_() {};

        /**
         * Read the HTTPRequest from the socket.
         * @param data The VirtualReadBuffer to read from.
         * @return The ParseResult.
         */
        virtual ParseResult parse( VirtualReadBuffer& data );

        virtual ParseResult parseBody( VirtualReadBuffer &data );

        virtual common::SystemError send( BaseSocket* socket );

        /**
         * Write the HTTPRequest to the socket.
         * @param socket The BaseSocket to write to.
         * @return The SystemError.
         */
        common::SystemError write( BaseSocket* socket ) const;


        /**
         * Get the HTTPRequestLine.
         * @return The HTTPRequestLine.
         */
        HTTPRequestLine& getRequestLine() { return request_line_; };


        /**
         * Set the HTTPRequestLine.
         * @param req The HTTPRequestLine to set.
         */
        void setRequestLine( const HTTPRequestLine& req );

        /**
         * Return the HTTPRequest as a string.
         * @return the HTTPRequest as a string.
         */
        virtual std::string asString() const;

        /**
         * Translate a Method enum to a human readable string.
         * @param method The method to translate.
         * @return the string.
         */
        static std::string methodAsString( Method method );

        /**
         * Translate an uppercase string ('GET', "POST', ..) into a Method enum.
         * @param s The uppercase string to translate.
         * @return the Method (and meINVALID if the string is not a Method).
         */
        static Method methodFromString( const std::string& s );

        /**
         * Return true if the Method allows a body in the message.
         * @param method the Method to check.
         * @return if the Method allows a body.
         */
        static bool methodAllowsBody( Method method );

      protected:
        /** The HTTPRequestLine of the HTTPRequest. */
        HTTPRequestLine request_line_;

    };

  }

}

#endif