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
 * @file stomp.cpp
 * Implements the dodo::network::stomp classes.
 */

#include "network/protocol/stomp/stomp.hpp"
#include "common/bytes.hpp"
#include "common/puts.hpp"

#include <cstring>


namespace dodo::network::protocol::stomp {

  Frame::FrameMatch Frame::readCommand( const common::Bytes& frame, size_t &index, const common::Bytes& command ) const {

    enum State {
      command_read,
      endofline,
    };

    State state = command_read;
    size_t cmd_idx = 0;
    while ( index < frame.getSize() ) {
      switch ( state ) {
        case command_read:
          if ( cmd_idx == command.getSize() ) {
            state = endofline;
          } else {
            if ( frame.getOctet(index) != command.getOctet(cmd_idx) ) return FrameMatch::NoMatch;
            index++;
            cmd_idx++;
          }
        case endofline:
          if ( frame.getOctet(index) == '\r' ) {
            index++;
          } else if ( frame.getOctet(index) == '\n' ) {
            return FrameMatch::FullMatch;
          } else return FrameMatch::NoMatch;
      }
    }
    return FrameMatch::IncompleteMatch;
  }

  Frame::FrameMatch Connect::match( const common::Bytes& frame, std::list<std::string> &errors ) const {
    size_t index = 0;
    Frame::FrameMatch match = readCommand( frame, index, command_connect );
    return match;
  }

  void Connect::generate( common::Bytes& frame ) const {
    frame.free();
    frame.append( command_connect );
    frame.append( eol );
    frame.append( header_accept_version_1_2 );
    frame.append( eol );
    frame.append( { common::Puts() << "host:" << host_ } );
    frame.append( eol );
    if ( login_.length() > 0 ) {
      frame.append( { common::Puts() << "login:" << login_ } );
      frame.append( eol );
      frame.append( { common::Puts() << "passcode:" << passcode_ } );
      frame.append( eol );
    }
    if ( heartbeat_out_ms_ ) {
      frame.append( { common::Puts() << "heart-beat:" << heartbeat_out_ms_ << "," << heartbeat_in_ms_ } );
      frame.append( eol );
    }

    frame.append( eol );
    frame.append( 0 );
  }

}
