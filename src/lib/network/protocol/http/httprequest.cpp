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
 * @file httprequest.cpp
 * Implements the dodo::network::protocolo::http::HTTPRequest class.
 */

#include <network/protocol/http/httprequest.hpp>

#include <common/util.hpp>

namespace dodo {

  namespace network {

    std::string HTTPRequest::methodAsString( Method method ) {
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

    HTTPRequest::Method HTTPRequest::methodFromString( const std::string& s ) {
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

    std::string HTTPRequest::HTTPRequestLine::asString() const {
      std::stringstream ss;
      ss << HTTPRequest::methodAsString( method_ ) << charSP;
      ss << request_uri_ << charSP;
      ss << http_version_.asString();
      ss << HTTPMessage::charCR << HTTPMessage::charLF;
      return ss.str();
    }

    std::string HTTPRequest::asString() const {
      std::stringstream ss;
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
      std::string method_string = "";
      std::string version_string = "";
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

    common::SystemError HTTPRequest::write( BaseSocket* socket ) const {
      return socket->send( this->asString().c_str(), this->asString().length(), false );
    }

  }

}