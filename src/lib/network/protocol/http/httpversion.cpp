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
 * @file httpversion.cpp
 * Implements the dodo::network::HTTPVersion class.
 */

#include <network/protocol/http/httpversion.hpp>

#include <common/util.hpp>

namespace dodo {

  namespace network {

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
      std::stringstream major;
      std::stringstream minor;
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
      if ( parseResult.getSystemError() == common::SystemError::ecOK ) {
        major >> major_;
        minor >> minor_;
      }
      return parseResult;
    }

  }

}