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
 * @file httpfragment.cpp
 * Implements the dodo::network::HTTPFragment class.
 */

#include <common/exception.hpp>
#include <common/puts.hpp>
#include <network/protocol/http/httpfragment.hpp>
#include <algorithm>
#include <sstream>
#include <iostream>

#include <common/util.hpp>

namespace dodo {

  namespace network::protocol::http {

    std::string HTTPFragment::getParseResultAsString( ParseError error ) {
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

  }

}