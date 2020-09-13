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

#include <network/tcpserver.hpp>
#include <common/logger.hpp>
#include <common/util.hpp>

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
      network::TCPListener::SocketWork* sockmap = 0;
      struct timeval now, last_snap;
      gettimeofday( &last_active_, NULL );
      gettimeofday( &now, NULL );
      snapRUsage();
      gettimeofday( &last_snap, NULL );
      state_ = ssWait;
      try {
        do {
          if ( listener_.waitForActivity( this ) ) {

            busy_ = true;
            gettimeofday( &last_active_, NULL );

            state_ = ssAwoken;

            do {

              sockmap = listener_.popWork();

              if ( sockmap ) {

                log_Debug( "TCPServer::run notify wakeup " <<
                           sockmap->socket->debugString() << " state " << sockmap->state );

                TCPListener::SockState completion_state = TCPListener::SockState::None;

                if ( sockmap->state & TCPListener::SockState::New ) {
                  log_Debug( "TCPServer::run ssNew " <<
                             sockmap->socket->debugString() << " state " << sockmap->state );
                  bool ok = false;
                  state_ = ssHandshake;
                  try {
                    ssize_t received = 0;
                    ssize_t sent = 0;
                    ok = handShake( sockmap->socket, received, sent );
                    listener_.addReceivedSentBytes( received, sent );
                    state_ = ssHandshakeDone;
                    completion_state |= TCPListener::SockState::New;
                    if ( !ok ) {
                      completion_state |= TCPListener::SockState::Shut;
                      log_Error( "TCPServer::run handshake failure socket " <<
                                sockmap->socket->debugString() );
                    }
                  }
                  catch ( std::exception &e ) {
                    log_Error( "TCPServer::run exception in handshake " << e.what()
                              << " socket " << sockmap->socket->debugString() );
                  }
                  catch ( ... ) {
                    log_Error( "TCPServer::run unhandled exception in handshake " <<
                              sockmap->socket->debugString() );
                  }
                }

                if ( sockmap->state & TCPListener::SockState::Read ) {
                  log_Debug( "TCPServer::run ssRead " <<
                             sockmap->socket->debugString() << " state " << sockmap->state );
                  bool ok = false;
                  state_ = ssReadSocket;
                  try {
                    ssize_t received = 0;
                    ssize_t sent = 0;

                    SystemError error = sockmap->data->readBuffer( sockmap->socket, received );
                    ok = (error == SystemError::ecOK);
                    error = readSocket( *sockmap, sent );
                    ok = ok && ( error == SystemError::ecOK || error == SystemError::ecEAGAIN );
                    listener_.addReceivedSentBytes( received, sent );
                    state_ = ssReadSocketDone;
                    completion_state |= TCPListener::SockState::Read;
                    if ( !ok ) {
                      completion_state |= TCPListener::SockState::Shut;
                      if ( error != SystemError::ecECONNABORTED ) {
                        log_Error( "TCPServer::run readSocket failure socket " <<
                                   sockmap->socket->getFD() << " client " <<
                                   sockmap->socket->getPeerAddress().asString(true) );
                        }
                    }
                  }
                  catch ( std::exception &e ) {
                     log_Error( "TCPServer::run exception in ssReadSocket " << e.what()
                                << " socket " << sockmap->socket->getFD() );
                  }
                  catch ( ... ) {
                     log_Error( "TCPServer::run unhandled exception in ssReadSocket socket " <<
                                sockmap->socket->getFD() );
                  }
                  state_ = ssReadSocketDone;
                }

                if ( sockmap->state & TCPListener::SockState::Shut ) {
                  log_Debug( "TCPServer::run ssShut " <<
                             sockmap->socket->debugString() << " state " << sockmap->state );
                  state_ = ssShutdown;
                  try {
                    shutDown( sockmap->socket );
                    state_ = ssShutdownDone;
                    completion_state |= TCPListener::SockState::Shut;
                  }
                  catch ( std::exception &e ) {
                     log_Error( "TCPServer::run exception in ssShutdown " << e.what()
                                << " socket " << sockmap->socket->debugString() );
                  }
                  catch ( ... ) {
                     log_Error( "TCPServer::run unhandled exception in ssShutdown " <<
                                sockmap->socket->debugString() );
                  }
                }
                state_ = ssReleaseWork;
                listener_.releaseWork( { sockmap->socket, completion_state } );
                state_ = ssReleaseWorkDone;
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
        log_Debug( "TCPServer::run stopping" );
      }
      catch ( const std::exception &e ) {
        log_Fatal( "TCPServer::run caught unhandled exception " << e.what() );
      }
      catch ( ... ) {
        log_Fatal( "TCPServer::run caught std::exception " );
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
