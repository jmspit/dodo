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
 * @file http.cpp
 * Implements the dodo::network::HTTPMessage class.
 */

#include <common/exception.hpp>
#include <common/puts.hpp>
#include <network/protocol/http/http.hpp>
#include <algorithm>
#include <sstream>
#include <iostream>

#include <common/util.hpp>

namespace dodo {

  namespace network {

    const char HTTPMessage::charCR = char(13);
    const char HTTPMessage::charLF = char(10);
    const char HTTPMessage::charSP = char(32);
    const char HTTPMessage::charHT = char(9);

    HTTPFragment::ParseResult HTTPVersion::parse( VirtualReadBuffer& buffer ) {
      ParseResult parseResult;
      enum State : uint8_t {
        ssStart,
        ssHttp,
        sshTtp1,
        sshTtp2,
        sshttP,
        ssSlash,
        ssMajor,
        ssDot,
        ssMinor,
        ssDone,
      };

      uint8_t state = ssStart;
      stringstream major;
      stringstream minor;
      while ( parseResult.ok() && state != ssDone ) {
        switch( state ) {
          case ssStart:
            if ( ! isspace( buffer.get() ) ) state = ssHttp;
            else parseResult.setSystemError( buffer.next() );
            break;
          case ssHttp:
            if ( buffer.get() == 'H' ) {
              parseResult.setSystemError( buffer.next() );
              state = sshTtp1;
            } else return { peInvalidHTTPVersion, common::SystemError::ecOK };
            break;
          case sshTtp1:
            if ( buffer.get() == 'T' ) {
              parseResult.setSystemError( buffer.next() );
              state = sshTtp2;
            } else return { peInvalidHTTPVersion, common::SystemError::ecOK };
            break;
          case sshTtp2:
            if ( buffer.get() == 'T' ) {
              parseResult.setSystemError( buffer.next() );
              state = sshttP;
            } else return { peInvalidHTTPVersion, common::SystemError::ecOK };
            break;
          case sshttP:
            if ( buffer.get() == 'P' ) {
              parseResult.setSystemError( buffer.next() );
              state = ssSlash;
            } else return { peInvalidHTTPVersion, common::SystemError::ecOK };
            break;
          case ssSlash:
            if ( buffer.get() == '/' ) {
              parseResult.setSystemError( buffer.next() );
              state = ssMajor;
            } else return { peInvalidHTTPVersion, common::SystemError::ecOK };
            break;
          case ssMajor:
            if ( !isdigit( buffer.get() ) ) state = ssDot;
            else {
              major << buffer.get();
              parseResult.setSystemError( buffer.next() );
            }
            break;
          case ssDot:
            if ( buffer.get() == '.' ) {
              parseResult.setSystemError( buffer.next() );
              state = ssMinor;
            }
            else return { peInvalidHTTPVersion, common::SystemError::ecOK };
            break;
          case ssMinor:
            if ( !isdigit( buffer.get() ) ) state = ssDone;
            else {
              minor << buffer.get();
              parseResult.setSystemError( buffer.next() );
            }
            break;
        }
      }
      if ( parseResult.getSystemError() == SystemError::ecOK ) {
        major >> major_;
        minor >> minor_;
      }
      return parseResult;
    }

    string HTTPFragment::getParseResultAsString( ParseError error ) {
      switch ( error ) {
        case peOk : return "Ok";
        case peIncomplete : return "incomplete fragment";
        case peSystemError : return "system error";
        case peExpectCRLF : return "malformed CRLF sequence";
        case peUnFinishedToken : return "malformed token";
        case peExpectingHeaderColon : return "expecting a ':'";
        case peInvalidHeaderFieldValue : return "malformed header field";
        case peInvalidHeaderListEnd : return "invalid header list";
        case peInvalidMethod : return "invalid request method";
        case peInvalidHTTPVersion : return "invalid HTTP version";
        case peInvalidRequestLine : return "invalid request line";
        case peInvalidContentLength : return "content-length mismatch";
        case peUnexpectedBody : return "body was given but not allowed";
        case peInvalidTransferEncoding : return "invalid transfer-encoding header";
        case peInvalidChunkHex : return "invalid hex chunk length";
        case peInvalidLastChunk : return "last chunk must have size 0";
        case peExpectingUnsignedInt : return "expecting an unsigned int";
        default : return common::Puts() << "unhandled error " << error;
      }
    }

    void HTTPMessage::addHeader( const string &key, const string &value ) {
      map<string,string>::const_iterator i = headers_.find( key );
      if ( i == headers_.end() ) {
        headers_[key] = value;
      } else {
        headers_[key] += "," + value;
      }
    }

    void HTTPMessage::replaceHeader( const string &key, const string &value ) {
        headers_[key] = value;
    }

    HTTPMessage::ParseResult HTTPMessage::parseHeaders( VirtualReadBuffer &data ) {
      ParseResult parseResult;
      parseResult.setSystemError( eatSpace( data ) );
      string header_key = "";
      string header_value = "";
      do {
        parseResult = parseToken( data, header_key );
        if ( parseResult.ok() ) {
          std::for_each( header_key.begin(), header_key.end(), [](char & c) {
            c = (char)std::tolower(c);
          });
          parseResult.setSystemError( eatSpace( data ) );
          if ( !parseResult.ok() ) return parseResult;
          if ( data.get() != ':' ) return { peExpectingHeaderColon, common::SystemError::ecOK };
          parseResult.setSystemError( eatSpace( data ) );
          if ( ! parseResult.ok() ) return parseResult;
          parseResult.setSystemError( data.next() );
          if ( ! parseResult.ok() ) return parseResult;
          parseResult = parseFieldValue( data, header_value );
          if ( parseResult.ok() || parseResult.eof() ) {
            addHeader( header_key, header_value );
            parseResult.setSystemError( eatSpace( data ) );
            if ( ! parseResult.ok() ) return parseResult;
          } else return parseResult;
        } else return parseResult;
      } while ( data.get() != charCR );
      parseResult.setSystemError( data.next() );
      if ( ! parseResult.ok() ) return parseResult;
      if ( data.get() != charLF ) return { peInvalidHeaderListEnd, common::SystemError::ecOK };
      return parseResult;
    }

    common::SystemError HTTPMessage::eatSpace( VirtualReadBuffer& buffer ) {
      common::SystemError error = common::SystemError::ecOK;
      while ( ( buffer.get() == charSP || buffer.get() == charHT ) && error == common::SystemError::ecOK ) {
        error = buffer.next();
      }
      return error;
    }

    HTTPMessage::ParseResult HTTPMessage::eatCRLF( VirtualReadBuffer& buffer ) {
      ParseResult parseResult;
      if ( buffer.get() != charCR ) return parseResult;
      parseResult.setSystemError( buffer.next() );
      if ( ! parseResult.ok() ) return parseResult;
      if ( buffer.get() != charLF ) return { peExpectCRLF, common::SystemError::ecOK };
      parseResult.setSystemError( buffer.next() );
      return parseResult;
    }

    HTTPMessage::ParseResult HTTPMessage::parseCRLF( VirtualReadBuffer& buffer ) {
      ParseResult parseResult;
      if ( buffer.get() != charCR ) return { peExpectCRLF, common::SystemError::ecOK };
      parseResult.setSystemError( buffer.next() );
      if ( ! parseResult.ok() ) return parseResult;
      if ( buffer.get() != charLF ) return { peExpectCRLF, common::SystemError::ecOK };
      parseResult.setSystemError( buffer.next() );
      return parseResult;
    }

    HTTPMessage::ParseResult HTTPMessage::parseChunkHex( VirtualReadBuffer& buffer, unsigned long &value ) {
      ParseResult parseResult;
      stringstream ss;
      while ( true ) {
        if ( std::isxdigit( buffer.get() ) ) {
          ss <<  buffer.get();
          parseResult.setSystemError( buffer.next() );
          if ( !parseResult.ok() ) return parseResult;
        } else break;
      }
      if ( ss.str().size() == 0 ) return { peInvalidChunkHex, common::SystemError::ecOK };
      ss >> std::hex >> value;
      return parseCRLF( buffer );
    }


    HTTPMessage::ParseResult HTTPMessage::parseFieldValue( VirtualReadBuffer& data, string &value ) {
      ParseResult parseResult;
      const int psStart = 0;
      const int psReadValue = 1;
      const int psCR = 2;
      const int psSP = 3;
      const int psDone = 101;
      value = "";
      int state = psStart;
      while ( parseResult.ok() && state != psDone ) {
        switch( state ) {
          case psStart:
            if ( !isSP( data.get() ) ) {
              state = psReadValue;
            } else parseResult.setSystemError( data.next() );
            break;
          case psReadValue:
            if ( data.get() == charCR ) {
              state = psCR;
              parseResult.setSystemError( data.next() );
            } else if ( isSP( data.get() ) ) {
              state = psSP;
              parseResult.setSystemError( data.next() );
            } else {
              value += data.get();
              parseResult.setSystemError( data.next() );
            }
            break;
          case psCR:
            if ( data.get() == charLF ) {
              parseResult.setSystemError( data.next() );
              if ( parseResult.ok() && isSP( data.get() ) ) {
                state = psSP;
                parseResult.setSystemError( data.next() );
              } else {
                state = psDone;
              }
            } else {
              return { peExpectCRLF, common::SystemError::ecOK };
            }
            break;
          case psSP:
            if ( isSP( data.get() ) ) {
              parseResult.setSystemError( data.next() );
            } else {
              value += charSP;
              state = psReadValue;
            }
            break;
        }
      }
      if ( state != psDone ) return { peInvalidHeaderFieldValue, common::SystemError::ecOK };
      return parseResult;
    }

    HTTPMessage::ParseResult HTTPMessage::parseToken( VirtualReadBuffer& buffer, string &token ) {
      ParseResult parseResult;
      token = "";
      std::stringstream stoken;
      while ( !isSeparator( buffer.get() ) && !isCTL( buffer.get() ) ) {
        stoken << buffer.get();
        parseResult.setSystemError( buffer.next() );
        if ( ! parseResult.ok() ) return parseResult;
      }
      if ( isSeparator( buffer.get() ) || isCTL( buffer.get() ) ) {
        token = stoken.str();
        return { peOk, common::SystemError::ecOK };
      } else return { peUnFinishedToken, common::SystemError::ecOK };
    }

    HTTPMessage::ParseResult HTTPMessage::parseUInt( VirtualReadBuffer &data, unsigned int& value ) {
      ParseResult parseResult;
      std::stringstream ss;
      value = 0;
      while ( parseResult.ok() && isdigit( data.get() ) ) {
        ss << data.get();
        parseResult.setSystemError( data.next() );
      }
      if ( !ss.str().length() ) return { peExpectingUnsignedInt, common::SystemError::ecOK };
      ss >> value;
      return parseResult;
    }

    HTTPMessage::ParseResult HTTPMessage::parseChunkedBody( VirtualReadBuffer &data ) {
      ParseResult parseResult;
      size_t chunk_size = 0;
      while ( parseResult.ok() ) {
        parseResult = parseChunkHex( data, chunk_size );
        if ( parseResult.ok() ) {
          if ( chunk_size == 0 ) break;
          for ( size_t i = 0; i < chunk_size; i++ ) {
            body_ += data.get();
            parseResult.setSystemError( data.next() );
            if ( ! parseResult.ok() ) return parseResult;
          }
          parseResult = parseCRLF( data );
          if ( ! parseResult.ok() ) return parseResult;
        } else return parseResult;
      }
      if ( chunk_size != 0 ) return { peInvalidLastChunk, common::SystemError::ecOK };
      parseResult = eatCRLF( data );
      if ( parseResult.eof() ) return { peOk, common::SystemError::ecOK }; else return { peInvalidLastChunk, common::SystemError::ecOK };
    }

    HTTPMessage::ParseResult HTTPMessage::parseBody( VirtualReadBuffer &data ) {
      ParseResult parseResult;
      size_t content_length;
      if ( getHeaderValue( "content-length", content_length ) ) {
        for ( size_t i = 0; i < content_length; i++ ) {
          body_ += data.get();
          if ( i < content_length -1 ) {
            parseResult.setSystemError( data.next() );
            if ( ! parseResult.ok() ) return parseResult;
          }
        }
      } else {
        string transfer_encoding;
        if ( getHeaderValue( "transfer-encoding", transfer_encoding ) ) {
          if ( transfer_encoding == "chunked" ) {
            parseResult = parseChunkedBody( data );
            if ( ! parseResult.ok() ) return parseResult;
          } else return { peInvalidTransferEncoding, common::SystemError::ecOK };
        } else return { peUnexpectedBody , common::SystemError::ecOK };
      }
      return parseResult;
    }

    void HTTPMessage::setBody( const string& body ) {
      body_ = body;
      replaceHeader( "content-length", common::Puts() << common::Puts::fixed() << body_.size() );
    }

    bool HTTPMessage::getHeaderValue( const string &key, unsigned long &value ) const {
      map<string,string>::const_iterator i = headers_.find( key );
      value = 0;
      if ( i != headers_.end() ) {
        size_t pos = 0;
        value = stoul( i->second, &pos );
        return pos==i->second.size();
      } else return false;
    }

    bool HTTPMessage::getHeaderValue( const string &key, string &value ) const {
      map<string,string>::const_iterator i = headers_.find( key );
      value = "";
      if ( i != headers_.end() ) {
        value = i->second;
        return true;
      } else return false;
    }

    string HTTPRequest::methodAsString( Method method ) {
      switch ( method ) {
        case meGET     : return "GET";
        case meHEAD    : return "HEAD";
        case mePOST    : return "POST";
        case mePUT     : return "PUT";
        case mePATCH   : return "PATCH";
        case meDELETE  : return "DELETE";
        case meCONNECT : return "CONNECT";
        case meOPTIONS : return "OPTIONS";
        case meTRACE   : return "TRACE";
        default        : return "INVALID_METHOD";
      }
    }

    HTTPRequest::Method HTTPRequest::methodFromString( const string& s ) {
      if ( s == "GET" )          return meGET;
      else if ( s == "HEAD" )    return meHEAD;
      else if ( s == "POST" )    return mePOST;
      else if ( s == "PUT" )     return mePUT;
      else if ( s == "PATCH" )   return mePATCH;
      else if ( s == "DELETE" )  return meDELETE;
      else if ( s == "CONNECT" ) return meCONNECT;
      else if ( s == "OPTIONS" ) return meOPTIONS;
      else if ( s == "TRACE" )   return meTRACE;
      else                       return meINVALID;
    }

    bool HTTPRequest::methodAllowsBody( Method method ) {
      switch ( method ) {
        case meGET:
        case meHEAD:
        case meDELETE:
        case meCONNECT:
        case meTRACE:
          return false;
        case mePOST:
        case mePUT:
        case mePATCH:
        case meOPTIONS:
          return true;
        default : return false;
      }
    }

    string HTTPRequest::HTTPRequestLine::asString() const {
      stringstream ss;
      ss << HTTPRequest::methodAsString( method_ ) << charSP;
      ss << request_uri_ << charSP;
      ss << http_version_.asString();
      ss << HTTPMessage::charCR << HTTPMessage::charLF;
      return ss.str();
    }

    string HTTPRequest::asString() const {
      stringstream ss;
      ss << request_line_.asString();
      for ( auto header : headers_ ) {
        ss << header.first << ": " << header.second << HTTPMessage::charCR << HTTPMessage::charLF;
      }
      ss << HTTPMessage::charCR << HTTPMessage::charLF;
      if ( methodAllowsBody(request_line_.getMethod()) && body_.size() > 0 ) {
        ss << body_;
      }
      return ss.str();
    }

    HTTPMessage::ParseResult HTTPRequest::parse( VirtualReadBuffer &data ) {
      ParseResult parseResult = request_line_.parse( data );
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

    SystemError HTTPRequest::write( BaseSocket* socket ) const {
      return socket->send( this->asString().c_str(), this->asString().length(), false );
    }

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

    string HTTPResponse::asString() const {
      stringstream ss;
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

    HTTPMessage::ParseResult HTTPRequest::HTTPRequestLine::parse( VirtualReadBuffer &data ) {
      enum ParseState {
        psMethodStart,
        psMethodEnd,
        psRequestURIStart,
        psRequestURIEnd,
        psHTTPVersionCR,
        psHTTPVersionLF,
        psHTTPVersionEnd,
        psError,
        psDone,
      };
      ParseState state = psMethodStart;
      string method_string = "";
      string version_string = "";
      request_uri_ = "";
      ParseResult parseResult;
      while ( state != psDone && state != psError && parseResult.ok() ) {
        switch ( state ) {

          case psMethodStart:
            if ( std::isspace( data.get() ) ) {
              state = psMethodEnd;
            } else {
              method_string += data.get();
              parseResult.setSystemError( data.next() );
              if ( ! parseResult.ok() ) return parseResult;
            };
            break;

          case psMethodEnd:
            parseResult.setSystemError( eatSpace( data ) );
            if ( ! parseResult.ok() ) return parseResult;
            state = psRequestURIStart;
            break;

          case psRequestURIStart:
            if ( std::isspace( data.get() ) ) {
              state = psRequestURIEnd;
            } else {
              request_uri_ += data.get();
              parseResult.setSystemError( data.next() );
              if ( ! parseResult.ok() ) return parseResult;
            }
            break;

          case psRequestURIEnd:
            parseResult = http_version_.parse( data );
            if ( parseResult.ok() ) {
              state = psHTTPVersionCR;
              parseResult.setSystemError( eatSpace( data ) );
            } else return parseResult;
            break;

          case psHTTPVersionCR:
            if ( data.get() == HTTPMessage::charCR ) {
              state = psHTTPVersionLF;
              parseResult.setSystemError( data.next() );
              if ( !parseResult.ok() ) return parseResult;
            } else {
              return { peExpectCRLF, common::SystemError::ecOK };
            }
            break;

          case psHTTPVersionLF:
            if ( data.get() == HTTPMessage::charLF ) {
              state = psDone;
              parseResult.setSystemError( data.next() );
              if ( !parseResult.ok() ) return parseResult;
            } else {
              return { peExpectCRLF, common::SystemError::ecOK };
            }
            break;

          default: throw_SystemException( common::Puts() <<
                                    "HTTPRequest::HTTPRequestLine::parse unhandled state " <<
                                    state, 0 );

        }
      }
      if ( state == psDone ) {
        method_ = methodFromString( method_string );
        if ( method_ == meINVALID ) return { peInvalidMethod, common::SystemError::ecOK };
      } else return { peInvalidRequestLine, common::SystemError::ecOK };
      return parseResult;
    }

    string HTTPResponse::HTTPResponseLine::asString() const {
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

    string HTTPResponse::HTTPCodeAsString( HTTPCode code ) {
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
