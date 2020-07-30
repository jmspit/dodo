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
 * @file tcpserver.hpp
 * Defines the dodo::network::TCPListener class.
 */

#ifndef network_tcpserver_hpp
#define network_tcpserver_hpp

#include "network/tcplistener.hpp"

namespace dodo {

  namespace network {

    /**
     * Used in conjunction with TCPListener to implement high speed, multithreaded TCP services.
     * Subclass TCPServer and implement virtual
     *
     *  - bool handShake(network::BaseSocket*, ssize_t&, ssize_t& )
     *  - bool readSocket( TCPListener::SocketWork &work, ssize_t &sent )
     *  - void shutDown( BaseSocket* )
     *  - TCPServer* addServer()
     *
     * Each TCPServer is a thread that consumes work from the TCPListener. There
     * can be multiple TCPServer threads consuming work together, and the TCPListener auto-scales TCPServers
     * depending on the load.
     *
     * A TCPServer is tied to a TCPListener, The TCPListener produces work to the pool of TCPServer, either
     *
     * - new sockets.
     * - readable sockets (incoming data present).
     * - shutdown sockets (The TCPListener is informing the pool of TCPServer objects the BaseSocket will disappear).
     *
     * At any given moment, each TCPServer is in one of the TCPServer::ServerState states.
     *
     * @see TCPListener
     *
     */
    class TCPServer : public threads::Thread {
      public:

        /**
         * The possible states of a TCPServer.
         * @see getState()
         */
        enum ServerState {
          ssWait             = 0,  /**< The TCPServer has entered waiting for activity or wait timeout. */
          ssAwoken           = 1,  /**< The TCPServer has woken up from a wait either due to an event or the wait timing out. */
          ssHandshake        = 2,  /**< The TCPServer is about to invoke handShake(BaseSocket*). */
          ssHandshakeDone    = 3,  /**< ssHandshake completed. */
          ssReadSocket       = 4, /**< The TCPServer is about to invoke readSocket(BaseSocket*). */
          ssReadSocketDone   = 5, /**< ssReadSocket completed. */
          ssShutdown         = 6, /**< The TCPServer is about to invoke shutdown(BaseSocket*). */
          ssShutdownDone     = 7,  /**< ssShutdown completed. */
          ssReleaseWork      = 8,  /**< The TCPServer is releasing the request */
          ssReleaseWorkDone  = 9,  /**< ssReleaseWork completed. */
        };

        /**
         * Construct a TCPServer for the TCPListener.
         * @param listener TCPListener.
         */
        TCPServer( TCPListener &listener );
        virtual ~TCPServer() {};

        /**
         * Run the TCPServer thread.
         * @return void
         */
        virtual void run();

        /**
         * Override to perform a protocol handshake. Note that the TCP handshake has already taken place,
         * and handShake() will only be called for new sockets.
         * @param socket A pointer to the socket to handshake. If this returns false,. the socket
         * will be closed.
         * @param received The number of bytes received by the call.
         * @param sent The number of bytes sent by the call.
         * @return true if the handshake succeeded.
         */
        virtual bool handShake( network::BaseSocket *socket, ssize_t &received, ssize_t &sent ) = 0;

        /**
         * Override to perform a request-response cycle.
         * @param work The work to perform.
         * @param sent The number of bytes sent by the call.
         * @return
         *
         *   - common::SystemError::ecOK if all is well, the request is received completely, it is handled and a response is sent.
         *   - common::SystemError::ecEAGAIN if all is well but the data in work.data is not a complete request yet.
         *   - common::SystemError::ecECONNABORTED if the socket needs to be closed (for example by client request).
         */
        virtual common::SystemError readSocket( TCPListener::SocketWork &work, ssize_t &sent ) = 0;

        /**
         * Override to perform a shutdown.
         * @param socket The socket to work on.
         * @return void
         */
        virtual void shutDown( network::BaseSocket *socket ) = 0;

        /**
         * Spawn a new TCPServer.
         * @return a pointer to a new TCPServer
         */
        virtual TCPServer* addServer() = 0;

        /**
         * Return a new TCPConnectionData pointer or override to return a descendant of TCPConnectionData.
         * @return the new TCPConnectionData*.
         */
        virtual TCPConnectionData* newConnectionData() const { return new TCPConnectionData(); }

        /**
         * Return true if the TCPServer has stopped working.
         * @return True if the TCPServer has stopped on request.
         */
        bool hasStopped() const { return has_stopped_; };

        /**
         * Return true if the TCPServer is busy.
         * @return True if busy.
         */
        bool isBusy() const { return busy_; };

        /**
         * Get the number of secodns the TCPServer has been idle.
         * @return The idle seconds.
         */
        double getIdleSeconds();

        /**
         * Set the request_stop_ flag, this call returns immediately, stopping will follow.
         */
        void requestStop() { request_stop_ = true; };

        /**
         * Return true if the TCPServer was requested to stop.
         * @return true if the TCPServer was requested to stop.
         */
        bool getRequestStop() const { return request_stop_; };

        /**
         * Return the current ServerState.
         * @return The current ServerState.
         */
        ServerState getState() const { return state_; };

      protected:

        /**
         * The TCPListener the TCPServer is working for.
         */
        TCPListener &listener_;

        /**
         * If true, the TCPServer should stop.
         */
        bool request_stop_;

        /**
         * If true, the TCPServer has stopped.
         */
        bool has_stopped_;

        /**
         * If true, the TCPServer is processing a request.
         */
        bool busy_;

        /**
         * Time of last request handling
         */
        struct timeval last_active_;

        /**
         * State of the TCPServer.
         */
        ServerState state_;
    };

  }

}

#endif
