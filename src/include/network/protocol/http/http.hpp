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
 * @file http.hpp
 * Defines the dodo::network::HTTPMessage, dodo::network::HTTPRequest amd dodo::network::HTTPResponse classes.
 */

#ifndef dodo_network_protocol_http_hpp
#define dodo_network_protocol_http_hpp

#include <map>
#include <network/basesocket.hpp>
#include <network/socketreadbuffer.hpp>
#include <string>

namespace dodo {

  namespace network {

    using namespace std;

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
        virtual string asString() const = 0;

        /**
         * Return the string description of a ParseError.
         * @param error The ParseError to describe.
         * @return The ParseError description.
         */
        static string getParseResultAsString( ParseError error );

    };

    /**
     * HHTP version comprising a major and minor number,
     * convertable from and to string as in HTTP requests.
     */
    class HTTPVersion : public HTTPFragment {
      public:
        /**
         * Default constructor to HTTP/1.0
         */
        HTTPVersion() : major_(1), minor_(0) {};

        /**
         * Construct from major and minor.
         * @param major the major to set.
         * @param minor the minor to set.
         */
        explicit HTTPVersion( unsigned int major, unsigned int minor ) : major_(major), minor_(minor) {};

        /**
         * Convert to string as 'HTTP/major_.minor_'
         * @return the HTTP VERSION string.
         */
        virtual string asString() const {
          stringstream ss;
          ss << fixed << "HTTP/" << major_ << "." << minor_;
          return ss.str();
        }

        /**
         * Construct from current position in the VirtualReadBuffer 'HTTP/major_.minor_'.
         * @param buffer the VirtualReadBuffer to read from.
         * @return the HTTPFragment::ParseError, peOk if no error.
         */
        virtual ParseResult parse( VirtualReadBuffer& buffer );

        /**
         * Return the major version.
         * @return The major version.
         */
        unsigned int getMajor() const { return major_; };

        /**
         * Return the minor version.
         * @return The minor version.
         */
        unsigned int getMinor() const { return minor_; };

        /**
         * Assignment operator.
         * @param version The version to assigne.
         * @return This HTTPVersion.
         */
        HTTPVersion& operator=( const HTTPVersion& version ) { major_ = version.major_; minor_ = version.minor_; return *this; };

        /**
         * Equality operator.
         * @param version The HTTPVersion to compare to.
         * @return True when equal.
         */
        bool operator==( const HTTPVersion& version ) const { return major_ == version.major_ && minor_ == version.minor_; };

        /**
         * Less-than operator.
         * @param version The HTTPVersion to compare to.
         * @return True if this < version.
         */
        bool operator<( const HTTPVersion& version ) const { return major_ < version.major_ ||
                                                                    (major_ == version.major_ && minor_ < version.minor_); };

        /**
         * Greater than operator.
         * @param version The HTTPVersion to compare to.
         * @return True if this > version.
         */
        bool operator>( const HTTPVersion& version ) const { return major_ > version.major_ ||
                                                                    (major_ == version.major_ && minor_ > version.minor_); };

      private:
        /** The major version. */
        unsigned int major_;
        /** The minor version. */
        unsigned int minor_;
    };

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
         * and this class transform the former into the latter automatically.
         * @param key The header key.
         * @param value The header value.
         */
        void addHeader( const string &key, const string &value );

        /**
         * Replace the value of a header.
         * @param key the key to replace.
         * @param value the value to set.
         */
        void replaceHeader( const string &key, const string &value );

        /**
         * Return a reference to the headers map.
         * @return A reference to headers_;
         */
        const map<string,string>& getHeaders() const { return headers_; };

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
        bool getHeaderValue( const string &key, string &value ) const;

        /**
         * Get a header value as an unsigned long into value.
         * @param key The header key.
         * @param value Receives the header value.
         * @return True if the key was found and the value an integer, false otherwise, in which case value is
         * undefined.
         */
        bool getHeaderValue( const string &key, unsigned long &value ) const;

        /**
         * Return the HTTTMessage body.
         * @return The HTTPMessage body.
         */
        const string& getBody() const { return body_; };

        /**
         * Set the body.
         * @param body The body to set.
         */
        void setBody( const string& body );


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
        ParseResult parseFieldValue( VirtualReadBuffer &data, string &value );

        /**
         * Parse a token (such as a header field name).
         * @param buffer The HTTP data source.
         * @param token The value receiving the token.
         * @return The ParseResult.
         */
        ParseResult parseToken( VirtualReadBuffer& buffer, string &token );

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
        ParseResult parseBody( VirtualReadBuffer &data );

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
        map<string,string> headers_;

        /**
         * The message body (if any).
         */
        string body_;

    };

    /**
     * HTTPRequest class. Contains
     * - A HTTPRequestLine
     * - An optional body
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
            virtual string asString() const;

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
            string getRequestURI() const { return request_uri_; };

            /**
             * Set the request uri.
             * @param uri The request uri.
             */
            void setRequestURI( const string &uri ) { request_uri_ = uri; };

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
            string              request_uri_;

            /**
             * The HTTP version.
             */
            HTTPVersion         http_version_;
        };

        HTTPRequest() : request_line_() {};

        /**
         * Read the HTTPRequest from the socket.
         * @param data The VirtualReadBuffer to read from.
         * @return The ParseResult.
         */
        virtual ParseResult parse( VirtualReadBuffer& data );

        /**
         * Write the HTTPRequest to the socket.
         * @param socket The BaseSocket to write to.
         * @return The SystemError.
         */
        SystemError write( BaseSocket* socket ) const;


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
        virtual string asString() const;

        /**
         * Translate a Method enum to a human readable string.
         * @param method The method to translate.
         * @return the string.
         */
        static string methodAsString( Method method );

        /**
         * Translate an uppercase string ('GET', "POST', ..) into a Method enum.
         * @param s The uppercase string to translate.
         * @return the Method (and meINVALID if the string is not a Method).
         */
        static Method methodFromString( const string& s );

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

    /**
     * HTTPResponse class. Contains
     * - A HTTPResponseLine
     * - An optional body
     */
    class HTTPResponse : public HTTPMessage {
      public:

        /**
         * @see https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
         */
        enum HTTPCode : unsigned int {
          // 1xx Informational response
          hcContinue           = 100,
          hcSwitchingProtocols = 101,
          hcProcessing         = 102,
          hcEarlyHints         = 103,

          // 2xx Success
          hcOK                        = 200,
          hcCreated                   = 201,
          hcAccepted                  = 202,
          hcNonAuthoritiveInformation = 203,
          hcNoContent                 = 204,
          hcResetContent              = 205,
          hcPartialContent            = 206,
          hcMultiStatus               = 207,
          hcAlreadyReported           = 208,
          hcIMUsed                    = 226,

          // 3xx Redirection
          hcMultipleChoices   = 300,
          hcMovedPermanently  = 301,
          hcFound             = 302,
          hcSeeOther          = 303,
          hcNotModified       = 304,
          hcUseProxy          = 305,
          hcSwitchProxy       = 306,
          hcTemporaryRedirect = 307,
          hcPermanentRedirect = 308,

          // 4xx Client errors
          hcBadRequest                  = 400,
          hcUnAuthorized                = 401,
          hcPaymentRequired             = 402,
          hcForbidden                   = 403,
          hcNotFound                    = 404,
          hcMethodNotAllowed            = 405,
          hcNotAcceptable               = 406,
          hcProxyAuthenticationRequired = 407,
          hcRequestTimeout              = 408,
          hcConflict                    = 409,
          hcGone                        = 410,
          hcLengthRequired              = 411,
          hcPreconditionFailed          = 412,
          hcPayloadTooLarge             = 413,
          hcURITooLong                  = 414,
          hcUnsupportedMediaType        = 415,
          hcRangeNotSatisfiable         = 416,
          hcExpectationFailed           = 417,
          hcIAmATeapot                  = 418,
          hcMisdirectRequest            = 421,
          hcUnporessableEntity          = 422,
          hcLocked                      = 423,
          hcFailedDependency            = 424,
          hcUpgradeRequired             = 426,
          hcPreconditionRequired        = 428,
          hcTooManyRequests             = 429,
          hcRequestHeaderFieldsTooLarge = 431,
          hcUnavailableForLegalReasons  = 451,

          // 5xx Server errors
          hcInternalServerError           = 500,
          hcNotImplemented                = 501,
          hcBadGateway                    = 502,
          hcServiceUnavailable            = 503,
          hcGatewayTimeout                = 504,
          hcHTTPVersionNotSupported       = 505,
          hcVariantAlsoNegotiates         = 506,
          hcInsufficientStorage           = 507,
          hcLoopDetected                  = 508,
          hcNotExtended                   = 510,
          hcNetworkAuthenticationRequired = 511,

          // Unofficial codes
          hcCheckpoint                       = 103,
          hcThisIsFine                       = 218,
          hcPageExpired                      = 419,
          hcMethodFailure                    = 420,
          hcEnhanceYourCalm                  = 420,
          hcBlockedByWindowsParentalControls = 450,
          hcInvalidToken                     = 498,
          hcTokenRequired                    = 499,
          hcBandwidthLimitExceeded           = 509,
          hcInvalidSSLCertificate            = 526,
          hcSiteIsFrozen                     = 530,
          hcNetworkReadTimeoutError          = 598,

          // Microsoft Internet Information Services
          hcLoginTimeout = 440,
          hcRetryWith    = 449,
          hcRedirect     = 451,

          // nginx
          hcNoRepsonse                 = 444,
          hcRequestHeaderTooLarge      = 494,
          hcSSLCertificateError        = 495,
          hcSSLCertificateRequired     = 496,
          hcHHTPRequestSentToHTTPSPort = 497,
          hcClientClosedrequest        = 499,

          // Cloudflare
          hcUnknownError        = 520,
          hcWebServerIsDown     = 521,
          hcConnectionTimedOut  = 522,
          hcOriginIsUnreachable = 523,
          hcATimeoutOccured     = 524,
          hcSSLHandshakeFailed  = 525,
          hcRailgunError        = 527,
          hcOriginDNSError      = 530,

        };

        /**
         * A HTTP response line.
         */
        class HTTPResponseLine : public HTTPFragment {
          public:

            /**
             * Default constructor.
             */
            HTTPResponseLine() : version_(), http_code_(hcOK) {};

            /**
             * Explicit consructor.
             * @param version The version to set.
             * @param http_code The HTTP code to set.
             */
            explicit HTTPResponseLine( const HTTPVersion &version, HTTPCode http_code ) : version_(version), http_code_(http_code) {};

            /**
             * Convert the HTTPResponseLine to a HTTP string.
             * @return The HTTPResponseLine as a string.
             */
            virtual string asString() const;

            virtual ParseResult parse( VirtualReadBuffer& data );

            /**
             * The HTTPVersion of the response line.
             * @return The HTTPVersion
             */
            const HTTPVersion& getHTTPVersion() const { return version_; };

            /**
             * The HTTP return code of the HTTPResponse.
             * @return The HTTPCode.
             */
            HTTPCode getHTTPCode() const { return http_code_; };

          private:
            /** The HTTPVersion. */
            HTTPVersion version_;
            /** The http_code */
            HTTPCode    http_code_;
        };

        const HTTPResponseLine& getResponseLine() const { return response_line_; };

        virtual ParseResult parse( VirtualReadBuffer& buffer );

        /**
         * Return the HTTPResponse as a string.
         * @return the HTTP response as a string.
         */
        virtual string asString() const;

        /**
         * Write the HTTPRequest to the socket.
         */
        void send( BaseSocket* socket );

        /**
         * Return the (upper case) string representation of the http error code.
         * @param code The HTTP error code.
         * @return The string representation.
         */
        static string HTTPCodeAsString( HTTPCode code );

      protected:
        HTTPResponseLine response_line_;

    };

  }

}

#endif
