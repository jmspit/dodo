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
 * @file httpresponse.cpp
 * Implements the dodo::network::protocolo::http::HTTPResponse class.
 */

#include <network/protocol/http/httpresponse.hpp>

#include <common/util.hpp>

namespace dodo {

  namespace network {

    HTTPMessage::ParseResult HTTPResponse::parse( VirtualReadBuffer &data ) {
      ParseResult parseResult = response_line_.parse( data );
      if ( parseResult.ok() ) {
        if ( data.get() == charCR ) {
          parseResult.setSystemError( data.next() );
          if ( parseResult.ok() && data.get() == charLF ) {
            parseResult.setSystemError( data.next() );
            if ( parseResult.ok() ) return parseBody( data ); else  return parseResult;
          } else return { peOk, common::SystemError::ecOK };
        } else {
          // these are headers
          parseResult = parseHeaders( data );
          if ( !parseResult.ok() ) return parseResult;
          if ( data.get() == charLF ) {
            parseResult.setSystemError( data.next() );
            if ( parseResult.ok() ) {
              return parseBody( data );
            } else if ( parseResult.eof() ) return { peOk, common::SystemError::ecOK };
            else return parseResult;
          } else return { peOk, common::SystemError::ecOK };
        }
      }
      return parseResult;
    }

    std::string HTTPResponse::asString() const {
      std::stringstream ss;
      ss << response_line_.asString();
      for ( auto header : headers_ ) {
        ss << header.first << ": " << header.second << HTTPMessage::charCR << HTTPMessage::charLF;
      }
      ss << HTTPMessage::charCR << HTTPMessage::charLF;
      if ( body_.size() > 0 ) {
        ss << body_;
      }
      return ss.str();
    }

    std::string HTTPResponse::HTTPResponseLine::asString() const {
      std::stringstream ss;
      ss << version_.asString();
      ss << charSP;
      ss << std::fixed << std::dec << http_code_;
      ss << charSP;
      ss << HTTPCodeAsString( http_code_ );
      ss << charCR;
      ss << charLF;
      return ss.str();
    }

    HTTPFragment::ParseResult HTTPResponse::HTTPResponseLine::parse( VirtualReadBuffer& data ) {
      ParseResult parseResult;
      parseResult = version_.parse( data );
      if ( parseResult.ok()) {
        parseResult.setSystemError( eatSpace( data ) );
        if ( !parseResult.ok() ) return parseResult;
        unsigned int i;
        parseResult = parseUInt( data, i );
        if ( parseResult.ok() ) {
          http_code_ = static_cast<HTTPCode>(i);
          // ignore the error string (we already have the HTTP code) but we do expect CRLF.
          while ( parseResult.ok() && data.get() != charCR ) {
            parseResult.setSystemError( data.next() );
            if ( ! parseResult.ok() ) return parseResult;
          }
          if ( data.get() != charCR )
            return { peExpectCRLF, common::SystemError::ecOK };
          else {
            parseResult.setSystemError( data.next() );
            if ( ! parseResult.ok() ) return parseResult;
          }
          if ( data.get() != charLF )
            return { peExpectCRLF, common::SystemError::ecOK };
          else {
            parseResult.setSystemError( data.next() );
            if ( parseResult.ok() ) return parseResult;
          }
        }
      }
      return parseResult;
    }

    std::string HTTPResponse::HTTPCodeAsString( HTTPCode code ) {
      switch ( code ) {

        case hcContinue           : return "CONTINUE";
        case hcSwitchingProtocols : return "SWITCHING PROTOCOLS";
        case hcProcessing         : return "PROCESSING";
        case hcEarlyHints         : return "EARLY HINTS";

        case hcOK                        : return "OK";
        case hcCreated                   : return "CREATED";
        case hcAccepted                  : return "ACCEPTED";
        case hcNonAuthoritiveInformation : return "NON-AUTHORITATIVE INFORMATION";
        case hcNoContent                 : return "NO CONTENT";
        case hcResetContent              : return "RESET CONTENT";
        case hcPartialContent            : return "PARTIAL CONTENT";
        case hcMultiStatus               : return "MULTI-STATUS";
        case hcAlreadyReported           : return "ALREADY REPORTED";
        case hcIMUsed                    : return "IM USED";

        case hcMultipleChoices   : return "MULTIPLE CHOICES";
        case hcMovedPermanently  : return "MOVED PERMANENTLY";
        case hcFound             : return "FOUND";
        case hcSeeOther          : return "SEE OTHER";
        case hcNotModified       : return "NOT MODIFIED";
        case hcUseProxy          : return "USE PROXY";
        case hcSwitchProxy       : return "SWITCH PROXY";
        case hcTemporaryRedirect : return "TEMPORARY REDIRECT";
        case hcPermanentRedirect : return "PERMANENT REDIRECT";

        case hcBadRequest                  : return "BAD REQUEST";
        case hcUnAuthorized                : return "UNAUTHORIZED";
        case hcPaymentRequired             : return "PAYMENT REQUIRED";
        case hcForbidden                   : return "FORBIDDEN";
        case hcNotFound                    : return "NOT FOUND";
        case hcMethodNotAllowed            : return "METHOD NOT ALLOWED";
        case hcNotAcceptable               : return "NOT ACCEPTABLE";
        case hcProxyAuthenticationRequired : return "PROXY AUTHENTICATION REQUIRED";
        case hcRequestTimeout              : return "REQUEST TIMEOUT";
        case hcConflict                    : return "CONFLICT";
        case hcGone                        : return "GONE";
        case hcLengthRequired              : return "LENGTH REQUIRED";
        case hcPreconditionFailed          : return "PRECONDITION FAILED";
        case hcPayloadTooLarge             : return "PAYLOAD TOO LARGE";
        case hcURITooLong                  : return "URI TOO LONG";
        case hcUnsupportedMediaType        : return "UNSUPPORTED MEDIA TYPE";
        case hcRangeNotSatisfiable         : return "RANGE NOT SATISFIABLE";
        case hcExpectationFailed           : return "EXPECTATION FAILED";
        case hcIAmATeapot                  : return "I'M A TEAPOT";
        case hcMisdirectRequest            : return "MISDIRECTED REQUEST";
        case hcUnporessableEntity          : return "UNPROCESSABLE ENTITY";
        case hcLocked                      : return "LOCKED";
        case hcFailedDependency            : return "FAILED DEPENDENCY";
        case hcUpgradeRequired             : return "UPGRADE REQUIRED";
        case hcPreconditionRequired        : return "PRECONDITION REQUIRED";
        case hcTooManyRequests             : return "TOO MANY REQUESTS";
        case hcRequestHeaderFieldsTooLarge : return "REQUEST HEADER FIELDS TOO LARGE";
        case hcUnavailableForLegalReasons  : return "UNAVAILABLE FOR LEGAL REASONS";

        case hcInternalServerError           : return "INTERNAL SERVER ERROR";
        case hcNotImplemented                : return "NOT IMPLEMENTED";
        case hcBadGateway                    : return "BAD GATEWAY";
        case hcServiceUnavailable            : return "SERVICE UNAVAILABLE";
        case hcGatewayTimeout                : return "GATEWAY TIMEOUT";
        case hcHTTPVersionNotSupported       : return "HTTP VERSION NOT SUPPORTED";
        case hcVariantAlsoNegotiates         : return "VARIANT ALSO NEGOTIATES";
        case hcInsufficientStorage           : return "INSUFFICIENT STORAGE";
        case hcLoopDetected                  : return "LOOP DETECTED";
        case hcNotExtended                   : return "NOT EXTENDED";
        case hcNetworkAuthenticationRequired : return "NETWORK AUTHENTICATION REQUIRED";

        case hcThisIsFine                       : return "THIS IS FINE";
        case hcPageExpired                      : return "PAGE EXPIRED";
        case hcMethodFailure                    : return "METHOD FAILURE";
        case hcBlockedByWindowsParentalControls : return "BLOCKED BY WINDOWS PARENTAL CONTROLS";
        case hcInvalidToken                     : return "INVALID TOKEN";
        //case hcTokenRequired                    : return "Token Required";
        case hcBandwidthLimitExceeded           : return "BANDWIDTH LIMIT EXCEEDED";
        case hcInvalidSSLCertificate            : return "INVALID SSL CERTIFICATE";
        //case hcSiteIsFrozen                     : return "Site is frozen";
        case hcNetworkReadTimeoutError          : return "NETWORK READ TIMEOUT ERROR";

        case hcLoginTimeout : return "LOGIN TIME-OUT";
        case hcRetryWith    : return "RETRY WITH";

        case hcNoRepsonse                 : return "NO RESPONSE";
        case hcRequestHeaderTooLarge      : return "REQUEST HEADER TOO LARGE";
        case hcSSLCertificateError        : return "SSL CERTIFICATE ERROR";
        case hcSSLCertificateRequired     : return "SSL CERTIFICATE REQUIRED";
        case hcHHTPRequestSentToHTTPSPort : return "HTTP REQUEST SENT TO HTTPS PORT";
        case hcClientClosedrequest        : return "CLIENT CLOSED REQUEST";

        case hcUnknownError        : return "UNKNOWN ERROR";
        case hcWebServerIsDown     : return "WEB SERVER IS DOWN";
        case hcConnectionTimedOut  : return "CONNECTION TIMED OUT";
        case hcOriginIsUnreachable : return "ORIGIN IS UNREACHABLE";
        case hcATimeoutOccured     : return "A TIMEOUT OCCURRED";
        case hcSSLHandshakeFailed  : return "SSL HANDSHAKE FAILED";
        case hcRailgunError        : return "RAILGUN ERROR";
        case hcOriginDNSError      : return "ORIGIN DNS ERROR";

        default : return "UNKNOWN HTTP CODE";
      }
    }

  }

}