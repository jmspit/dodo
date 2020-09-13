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
 * @file tcplistener.hpp
 * Defines the dodo::network::TCPListener class.
 */

#ifndef network_tcplistener_hpp
#define network_tcplistener_hpp

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <stdint.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <yaml-cpp/yaml.h>

#include "common/exception.hpp"
#include "common/octetarray.hpp"
#include "network/socket.hpp"
#include "threads/mutex.hpp"
#include "threads/thread.hpp"


namespace dodo {

  namespace network {

    using namespace std;
    using namespace common;

    class TCPServer;
    class TCPListenerTimer;

    /**
     * Class to track a connection.
     */
    class TCPConnectionData {
      public:

        /**
         * Construct
         */
        TCPConnectionData() {};

        /**
         * Destruct.
         */
        virtual ~TCPConnectionData() {};

        /**
         * Reads and appends data to read_buffer
         * @param socket The socket to read from.
         * @param received The number of bytes received.
         * @return The number of bytes read.
         */
        SystemError readBuffer( BaseSocket* socket, ssize_t &received );

        /**
         * Clears the read_buffer.
         */
        void clearBuffer();

        /**
         * Get a reference to the read buffer.
         * @return a reference to the read buffer.
         */
        const common::OctetArray& getReadBuffer() const { return read_buffer; }

      protected:
        /** Buffer for (incomplete) request data. */
        common::OctetArray read_buffer;
    };

    /**
     *
     * The TCPListener listens, accepts connections and generates socket events to produce TCP work to a pool of
     * TCPServer worker objects. Developers must implement a protocol by subclassing TCPServer. The produced
     * by a TCPListener is either
     *
     *   - handling new connections (and optionally  perform a protocol handShake).
     *   - reading data from sockets and sending responses
     *   - closing down connections on hangups or errors.
     *
     * The developer sub-classes TCPServer to implement a request-response protocol.
     *
     * TCPListener uses the Linux epoll_wait interface to pick up socket events and distribute the work to TCPServers.
     *
     * The TCPListener is started start( TCPServer *server ) with an initial TCPServer object. The
     * TCPServer::addServer() method is used to spawn additional TCPServer objects up to Params.minservers when
     * TCPListener::run() is invoked. The TCPListener will spawn TCPServer worker threads if the amount of pending
     * work exceeds the number of existing TCPServer objects, up to Params.maxservers.
     *
     * The TCPServer descendant class implements the details of handShake, readSocket and shutDown, and are each
     * passed a SocketWork that needs to be serviced -a SocketWork specifies the socket and the event state it is in.
     *
     * Sockets are set to non-blocking mode, and TCPServer::readSocket is not guaranteed to see a full request, data
     * might arrive in multiple chunks.
     *
     * For a given connection, a TCPServer descendant will cycle through
     *
     *   - a handShake call
     *   - zero or more readSocket calls
     *   - a shutDown call
     *
     * On connection errors and hangups call to shutDown will follow - the BaseSocket object passed to shutDown() is
     * not destroyed yet, allowing TCPServer implementations to clean up failed connections.
     */
    class TCPListener : public threads::Thread {
      public:

        /**
         * Parameters affecting TCPListener behavior.
         */
        struct Params {

          /**
           * Construct with default parameters.
           */
          Params() :
            minservers(8),
            maxservers(16),
            maxconnections(6000),
            maxqdepth(128),
            sendbufsz(16384),
            recvbufsz(32768),
            server_idle_ttl_s(300),
            pollbatch(128),
            listener_sleep_ms(1000),
            throttle_sleep_us(4000),
            cycle_max_throttles(40),
            stat_trc_interval_s(300),
            send_timeout_seconds(10),
            receive_timeout_seconds(10),
            tcp_keep_alive(false)
            {};

          /**
           * Construct from a YAML::Node.
           * @param node The YAML::Node to read parameters from.
           */
          Params( const YAML::Node &node );

          /**
           * The minimum number of TCPServers
           */
          size_t minservers;

          /**
           * The maximum number of TCPServers
           */
          size_t maxservers;

          /**
           * The maximum number of connections (in effect, sockets) allowed.
           */
          size_t maxconnections;

          /**
           * Maximum size of connection and request work queues.
           */
          size_t maxqdepth;

          /**
           * The send buffer size, set on the listening socket, inherited by accepted sockets.
           */
          socklen_t sendbufsz;

          /**
           * The receive buffer size, set on the listening socket, inherited by accepted sockets.
           */
          socklen_t recvbufsz;

          /**
           * The maximum TCPServer idle time in seconds before stopping the server, honoring minservers.
           */
          double server_idle_ttl_s;

          /**
           * The number of epoll_wait events read in one epol__wait wake-up
           */
          int pollbatch;

          /**
           * The TCPListener epoll_wait timeout in ms
           */
          int listener_sleep_ms;

          /**
           * The time to sleep when the queue gets too big (letting workers clear from the queue without
           * accepting new work.
           */
          size_t throttle_sleep_us;

          /**
           * Maximum number of throttles per epoll_wait cycle. The listener thread is blocked no longer than
           * cycle_max_throttles * throttle_sleep_us per epoll_wait cycle.
           */
          size_t cycle_max_throttles;

          /**
           * The epol__wait timeout in ms
           */
          time_t stat_trc_interval_s;

          /**
           * Send timeout in seconds.
           */
          int send_timeout_seconds;

          /**
           * Receive timeout in seconds.
           */
          int receive_timeout_seconds;

          /**
           * Toggle TCP keep-alive.
           */
          bool tcp_keep_alive;
        };


        /**
         * Load statisics.
         */
        struct Stats {
          Stats() : connections(0), requests(0), throttles(0),received(0),sent(0) {};
          /** The number of connections. */
          ssize_t connections;
          /** The number of requests. */
          ssize_t requests;
          /** The number of throttles. */
          ssize_t throttles;
          /** The number of bytes received */
          ssize_t received;
          /** The number of bytes sent */
          ssize_t sent;
        };

        /**
         * BaseSocket lifecycle states.
         */
        enum class SockState {
          None   = 0,     /**< Undefined / initial */
          New    = 1,     /**< New connection, TCPServer::handShake() will be called */
          Read   = 2,     /**< Data is ready to be read, TCPServer::readSocket() will be called */
          Shut   = 4      /**< BaseSocket is hung up or in error, TCPServer::shutDown() will be called */
        };

        /**
         * BaseSocket socket and state pair.
         * @see clients_
         */
        struct SocketWork {
          /** Pointer to the socket */
          BaseSocket* socket = nullptr;
          /** State of the socket */
          SockState state = SockState::New;
          /** Data context */
          TCPConnectionData* data = nullptr;
        };

        /**
         * Constructor
         * @param address The address to listen on.
         * @param params The Params to use.
         * @see listen()
         */
        TCPListener( const Address& address, const Params &params );

        /**
         * Constructor, read Address and params from the YAML::Node.
         * @param yaml The YAML::NOde to read the configuration from.
         */
        TCPListener( const YAML::Node &yaml );

        ~TCPListener();

        /**
         * Start the TCPListener and initialize with a TCPServer implementation object.
         * @param server A TCPServer descendant as initial server.
         */
        void start( TCPServer *server );

        /**
         * Request a stop on the TCPListener. Call wait() to wait for the thread to actually be stopped.
         */
        void stop() { stop_server_ = true; }

      private:

        /**
         * Add an epoll event for the BaseSocket.
         * @param s The BaseSocket
         * @param events The events to set (see man epoll_ctl).
         */
        void pollAdd( const BaseSocket* s, uint32_t events );

        /**
         * Modify an epoll event for the BaseSocket.
         * @param s The BaseSocket
         * @param events The events to set (see man epoll_ctl).
         */
        void pollMod( const BaseSocket* s, uint32_t events );

        /**
         * Delete the epoll event for the BaseSocket.
         * @param s The BaseSocket
         */
        void pollDel( const BaseSocket* s );

        /**
         * Log TCPListener statistics to Logger::getLogger().
         */
        void logStats();

        /**
         * Throttle the listener when work_q_sz_ exceeds params_.maxqdepth
         */
        void throttle();

        /**
         * Add a server if work_q_sz_ exceeds servers.size() and servers_.size() < params_.maxservers
         */
        void addServers();

        /**
         * Cleanup stopped TCPServers
         */
        void cleanStoppedServers();

        /**
         * Tell the listener to close the socket.
         * @param socket The socket to close.
         */
        void closeSocket( BaseSocket *socket );

        /**
         * Called by the TCPServer, enters wait state until woken up by either a timeout or a notify
         * by TCPListener that there are 1 or more requests to be handled.
         * @param server The TCPServer calling this.
         * @return True if there are pending requests or request_stop_ is true.
         */
        bool waitForActivity( TCPServer* server );

        /**
         * Common constructor code.
         * @param address The address to listen on.
         * @param params The Params to use.*
         */
        void construct( const Address& address, const Params &params );

        /**
         * Push work.
         * @param work The SocketWork to handle.
         * @see SocketWork.
         */
        //void pushWork( BaseSocket* socket, SockState state );
        void pushWork( const SocketWork &work );

        /**
         * Push work.
         * @param fd The BaseSocket fieldescriptor.
         * @param state The state the BaseSocket is in.
         * @see SockState.
         */
        void pushWork( int fd, SockState state );

        /**
         * Pop work.
         * @return A SocketWork* to handle, or NULL if there is no work to handle.
         */
        SocketWork* popWork();

        /**
         * Called by a TCPServer to signal that the work has been handled and event detection on it can resume.
         * @param work The SocketWork.
         */
        void releaseWork( const SocketWork &work );

        /**
         * Entrypoint, override of Thread::run()
         * @return void
         */
        virtual void run();

        /**
         * Add received bytes.
         * @param r The received bytes to add.
         * @param s The sent bytes to add
         */
        void addReceivedSentBytes( ssize_t r, ssize_t s ) {
          threads::Mutexer lock( stats_mutex_ );
          last_stats_.received += r;
          last_stats_.sent += s;
        }

        /**
         * The listening address.
         */
        Address listen_address_;

        /**
         * TCPListener parameters
         */
        Params params_;

        /**
         * The initial TCPServer, which will live as long as the TCPListener runs, and is sued
         * to create new TCPServers under load.
         */
        TCPServer* init_server_;

        /**
         * The listening BaseSocket.
         */
        BaseSocket* listen_socket_;

        /**
         * The backlog (incoming connection queue before accept calls clearing it) used by listen.
         * @see man 2 listen
         */
        int backlog_;

        /**
         * If true, the TCPListener will gracefully stop and finish.
         */
        bool stop_server_;

        /**
         * Protects both client_ and workload_
         */
        threads::Mutex clientmutex_;

        /**
         * Mutex used to wakeup TCPServer threads with condition variable cv_signal_.
         * @see cv_signal_.
         */
        std::mutex mt_signal_;

        /**
         * Condition variable used to wakeup TCPServer threads with mutex mt_signal_.
         * @see mt_signal_.
         */
        std::condition_variable cv_signal_;

        /**
         * Map of file descriptors to SocketWork for all connected clients. The work is either in progress or last
         * completed state.
         */
        map<int,SocketWork> clients_;

        /**
         * List of TCPServers.
         */
        std::list<TCPServer*> servers_;

        /**
         * Queue of sockets with work for TCPServer instances.
         */
        deque<SocketWork*> workload_;

        /**
         * The number of queued work items.
         */
        std::atomic<unsigned long long> work_q_sz_;

        /**
         * The epoll interface file descriptor.
         */
        int epoll_fd_;

        /**
         * The event mask for read events. This is the one-shot event mask for incoming data.
         */
        uint32_t read_event_mask_;

        /**
         * The event mask for hangup events. This is the event mask for all error and hangup events, an event mode
         * active whilst a read event for the socket is already queued for processing, so that errors may be caught
         * without clearing any pending read events on the socket.
         */
        uint32_t hangup_event_mask_;

        /**
         * The previous Stats.
         */
        Stats prev_stats_;

        /**
         * The last Stats.
         */
        Stats last_stats_;

        /**
         * Protects prev_stats_, last_stats_.
         */
        threads::Mutex stats_mutex_;

        /**
         * Time of last queueing warning
         */
        struct timeval warn_queue_time_;

        /**
         * Time of previous statistics
         */
        struct timeval prev_stat_time_;

        /**
         * Time of last statistics
         */
        struct timeval stat_time_;

        /**
         * Time of last check for stopped servers
         */
        struct timeval server_stopped_check_time_;

        /**
         * Synchronize access to now_.
         */
        threads::Mutex now_mutex_;

        /**
         * Frequently updated by a TCPListenerTimer with enough precision for its use.
         */
        struct timeval now_;

        /**
         * The number of times a connection was closed after accept because params_.maxconnectctions was reached.
         */
        size_t event_maxconnections_reached_;

        /**
         * TCPListenerTimer needs access to now_ and now_mutex_
         */
        friend class TCPListenerTimer;

        /**
         * TCPServer needs access to several private definitions, data and methods.
         */
        friend class TCPServer;

        friend TCPListener::SockState& operator|=( TCPListener::SockState&, TCPListener::SockState );
        friend TCPListener::SockState& operator^=( TCPListener::SockState&, TCPListener::SockState );
        friend bool operator&( const TCPListener::SockState&, const TCPListener::SockState& );

        friend const dodo::common::Puts& operator<<( const dodo::common::Puts&, TCPListener::SockState );

    };

    /**
     * Bitwise |= on SocketWork
     * @param self the lvalue
     * @param other the rvalue
     * @return the lvalue
     */
    inline TCPListener::SockState& operator|=( TCPListener::SockState &self, TCPListener::SockState other ) {
      return self = static_cast<TCPListener::SockState>( static_cast<int>(self) | static_cast<int>(other) );
    }

    /**
     * Bitwise ^= on SocketWork
     * @param self the lvalue
     * @param other the rvalue
     * @return the lvalue
     */
    inline TCPListener::SockState& operator^=( TCPListener::SockState &self, TCPListener::SockState other ) {
      return self = static_cast<TCPListener::SockState>( static_cast<int>(self) ^ static_cast<int>(other) );
    }

    /**
     * Bitwise & on SocketWork
     * @param self the lvalue
     * @param other the rvalue
     * @return the lvalue
     */
    inline bool operator&( const TCPListener::SockState &self, const TCPListener::SockState &other ) {
      return static_cast<int>(self) & static_cast<int>(other);
    }


    /**
     * Nice-write to a Puts.
     * @param os The output Puts
     * @param state The TCPListener::SockState to write.
     * @return The output Puts
     */
    inline const dodo::common::Puts& operator<<( const dodo::common::Puts& os, TCPListener::SockState state ) {
      if ( state & TCPListener::SockState::None ) os << "None|";
      if ( state & TCPListener::SockState::New ) os << "New|";
      if ( state & TCPListener::SockState::Read ) os << "Read|";
      if ( state & TCPListener::SockState::Shut ) os << "Shut|";
      //switch ( state ) {
        //case TCPListener::SockState::None :
          //os << "none";
          //break;
        //case TCPListener::SockState::New :
          //os << "new";
          //break;
        //case TCPListener::SockState::Read :
          //os << "read";
          //break;
        //case TCPListener::SockState::Shut :
          //os << "shut";
          //break;
      //}
      os << " (" << static_cast<int>(state) << ")";
      return os;
    }

  }

}

#endif
