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
 * @file httpresponse.hpp
 * Defines the dodo::network::protocol::http::HTTPResponse class.
 */

#ifndef dodo_network_protocol_http_httpresponse_hpp
#define dodo_network_protocol_http_httpresponse_hpp

#include <network/protocol/http/httpmessage.hpp>
#include <network/protocol/http/httpversion.hpp>
#include <string>

namespace dodo {

  namespace network::protocol::http {

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
            virtual std::string asString() const;

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

        virtual ParseResult parseBody( VirtualReadBuffer &data );

        virtual common::SystemError send( BaseSocket* socket );

        bool hasBody() const;

        /**
         * Return the HTTPResponse as a string.
         * @return the HTTP response as a string.
         */
        virtual std::string asString() const;

        /**
         * Return the (upper case) string representation of the http error code.
         * @param code The HTTP error code.
         * @return The string representation.
         */
        static std::string HTTPCodeAsString( HTTPCode code );

      protected:
        HTTPResponseLine response_line_;

    };

  }

}

#endif