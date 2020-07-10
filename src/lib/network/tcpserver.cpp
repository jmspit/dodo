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
 * @file tcpserver.cpp
 * Implements the dodo::network::TCPServer class.
 */

#include "network/tcpserver.hpp"
#include "common/logger.hpp"
#include "common/util.hpp"

#include <chrono>
#include <thread>
#include <cmath>


namespace dodo {

  namespace network {

    using namespace std;

    TCPServer::TCPServer( TCPListener &listener ) :
      listener_(listener),
      request_stop_(false),
      has_stopped_(false),
      busy_(false) {
      gettimeofday( &last_active_, NULL );
    }

    void TCPServer::run() {
      has_stopped_ = false;
      network::TCPListener::SockMap* sockmap = 0;
      struct timeval now, last_snap;
      gettimeofday( &last_active_, NULL );
      gettimeofday( &now, NULL );
      snapRUsage();
      gettimeofday( &last_snap, NULL );
      state_ = ssWait;
      try {
        do {
          if ( listener_.waitForActivity( this ) ) {
            Logger::getLogger()->debug( common::Puts() << "TCPServer::run waitForActivity sees activity" );

            busy_ = true;
            gettimeofday( &last_active_, NULL );

            state_ = ssAwoken;

            do {

              sockmap = listener_.popRequest();

              if ( sockmap ) {

                Logger::getLogger()->debug( common::Puts() << "TCPServer::run notify wakeup " <<
                                            sockmap->pointer->debugString() << " state " << sockmap->state );

                TCPListener::SockState completion_state = TCPListener::SockState::None;

                if ( sockmap->state & TCPListener::SockState::New ) {
                  Logger::getLogger()->debug( common::Puts() << "TCPServer::run ssNew " <<
                                              sockmap->pointer->debugString() << " state " << sockmap->state );
                  bool ok = false;
                  state_ = ssHandshake;
                  try {
                    ok = handShake( sockmap->pointer );
                    state_ = ssHandshakeDone;
                    completion_state |= TCPListener::SockState::New;
                    if ( !ok ) {
                      completion_state |= TCPListener::SockState::Shut;
                      Logger::getLogger()->error( common::Puts() <<
                                                  "TCPServer::run handshake failure socket " <<
                                                  sockmap->pointer->debugString() );
                    }
                  }
                  catch ( std::exception &e ) {
                     Logger::getLogger()->error( common::Puts() <<
                                                 "TCPServer::run exception in handshake " << e.what()
                                                 << " socket " << sockmap->pointer->debugString() );
                  }
                  catch ( ... ) {
                     Logger::getLogger()->error( common::Puts() <<
                                                 "TCPServer::run unhandled exception in handshake " <<
                                                 sockmap->pointer->debugString() );
                  }
                }

                if ( sockmap->state & TCPListener::SockState::Read ) {
                  Logger::getLogger()->debug( common::Puts() << "TCPServer::run ssRead " <<
                                              sockmap->pointer->debugString() << " state " << sockmap->state );
                  bool ok = false;
                  state_ = ssRequestResponse;
                  try {
                    ok = requestResponse( sockmap->pointer );
                    state_ = ssRequestResponseDone;
                    completion_state |= TCPListener::SockState::Read;
                    if ( !ok ) {
                      completion_state |= TCPListener::SockState::Shut;
                      Logger::getLogger()->error( common::Puts() <<
                                                  "TCPServer::run request-response failure socket " <<
                                                  sockmap->pointer->getFD() << " client " <<
                                                  sockmap->pointer->getPeerAddress().asString(true) );
                    }
                  }
                  catch ( std::exception &e ) {
                     Logger::getLogger()->error( common::Puts() <<
                                                 "TCPServer::run exception in handshake " << e.what()
                                                 << " socket " << sockmap->pointer->getFD() );
                  }
                  catch ( ... ) {
                     Logger::getLogger()->error( common::Puts() <<
                                                 "TCPServer::run unhandled exception in handshake socket " <<
                                                 sockmap->pointer->getFD() );
                  }
                  state_ = ssRequestResponseDone;
                }

                if ( sockmap->state & TCPListener::SockState::Shut ) {
                  Logger::getLogger()->debug( common::Puts() << "TCPServer::run ssShut " <<
                                              sockmap->pointer->debugString() << " state " << sockmap->state );
                  state_ = ssShutdown;
                  try {
                    shutDown( sockmap->pointer );
                    state_ = ssShutdownDone;
                    completion_state |= TCPListener::SockState::Shut;
                  }
                  catch ( std::exception &e ) {
                     Logger::getLogger()->error( common::Puts() <<
                                                 "TCPServer::run exception in handshake " << e.what()
                                                 << " socket " << sockmap->pointer->debugString() );
                  }
                  catch ( ... ) {
                     Logger::getLogger()->error( common::Puts() <<
                                                 "TCPServer::run unhandled exception in handshake " <<
                                                 sockmap->pointer->debugString() );
                  }
                }
                state_ = ssReleaseRequest;
                listener_.releaseRequest( sockmap->pointer, completion_state );
                state_ = ssReleaseRequestDone;
              }

            } while ( sockmap );

            busy_ = false;
            state_ = ssWait;
          }
          gettimeofday( &now, NULL );
          if ( common::getSecondDiff( last_snap, now ) > 0.2 ) {
            snapRUsage();
            gettimeofday( &last_snap, NULL );
          }

        } while ( !request_stop_ );
        Logger::getLogger()->debug( common::Puts() << "TCPServer::run stopping" );
      }
      catch ( const std::exception &e ) {
        Logger::getLogger()->fatal( common::Puts() <<
                                   "TCPServer::run caught unhandled exception " <<
                                   e.what() );
      }
      catch ( ... ) {
        Logger::getLogger()->fatal( common::Puts() <<
                                   "TCPServer::run caught std::exception " );
      }
      has_stopped_ = true;
    }

    double TCPServer::getIdleSeconds() {
      struct timeval tv;
      gettimeofday( &tv, 0 );
      return getSecondDiff( last_active_, tv );
    }

  }

}
