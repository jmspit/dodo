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
 * @file httpmessxage.cpp
 * Implements the dodo::network::protocolo::http::HTTPMessage class.
 */

#include <network/protocol/http/httpmessage.hpp>

#include <common/util.hpp>

namespace dodo {

  namespace network {

    const char HTTPMessage::charCR = char(13);
    const char HTTPMessage::charLF = char(10);
    const char HTTPMessage::charSP = char(32);
    const char HTTPMessage::charHT = char(9);

    void HTTPMessage::addHeader( const std::string &key, const std::string &value ) {
      std::map<std::string,std::string>::const_iterator i = headers_.find( key );
      if ( i == headers_.end() ) {
        headers_[key] = value;
      } else {
        headers_[key] += "," + value;
      }
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

    common::SystemError HTTPMessage::eatSpace( VirtualReadBuffer& buffer ) {
      common::SystemError error = common::SystemError::ecOK;
      while ( ( buffer.get() == charSP || buffer.get() == charHT ) && error == common::SystemError::ecOK ) {
        error = buffer.next();
      }
      return error;
    }

    bool HTTPMessage::getHeaderValue( const std::string &key, unsigned long &value ) const {
      std::map<std::string,std::string>::const_iterator i = headers_.find( key );
      value = 0;
      if ( i != headers_.end() ) {
        size_t pos = 0;
        value = stoul( i->second, &pos );
        return pos==i->second.size();
      } else return false;
    }

    bool HTTPMessage::getHeaderValue( const std::string &key, std::string &value ) const {
      std::map<std::string,std::string>::const_iterator i = headers_.find( key );
      value = "";
      if ( i != headers_.end() ) {
        value = i->second;
        return true;
      } else return false;
    }

    HTTPMessage::ParseResult HTTPMessage::parseHeaders( VirtualReadBuffer &data ) {
      ParseResult parseResult;
      parseResult.setSystemError( eatSpace( data ) );
      std::string header_key = "";
      std::string header_value = "";
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
      std::stringstream ss;
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


    HTTPMessage::ParseResult HTTPMessage::parseFieldValue( VirtualReadBuffer& data, std::string &value ) {
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

    HTTPMessage::ParseResult HTTPMessage::parseToken( VirtualReadBuffer& buffer, std::string &token ) {
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
        std::string transfer_encoding;
        if ( getHeaderValue( "transfer-encoding", transfer_encoding ) ) {
          if ( transfer_encoding == "chunked" ) {
            parseResult = parseChunkedBody( data );
            if ( ! parseResult.ok() ) return parseResult;
          } else return { peInvalidTransferEncoding, common::SystemError::ecOK };
        } else return { peUnexpectedBody , common::SystemError::ecOK };
      }
      return parseResult;
    }

    void HTTPMessage::replaceHeader( const std::string &key, const std::string &value ) {
        headers_[key] = value;
    }

    void HTTPMessage::setBody( const std::string& body ) {
      body_ = body;
      replaceHeader( "content-length", common::Puts() << common::Puts::fixed() << body_.size() );
    }

  }

}