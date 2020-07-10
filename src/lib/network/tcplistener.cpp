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
 * @file tcplistener.cpp
 * Implements the dodo::network::TCPListener class.
 */

#include "network/tcplistener.hpp"
#include "network/tcpserver.hpp"
#include "common/logger.hpp"
#include "common/util.hpp"

#include <algorithm>
#include <map>


namespace dodo {

  namespace network {

    using namespace std;

      /**
       * Updates the attribute now_ in the TCPListener at a regular interval to avoid excessive number of calls
       * to gettimeofday in the TCPListener event loop where time high time precision is of lesser importance.
       */
      class TCPListenerTimer : public threads::Thread {
        public:
          /**
           * Constructor, specify the TCPListener to operate on.
           * @param listener the TCPListener to operate on.
           */
          TCPListenerTimer( TCPListener &listener ) : listener_(listener), stopped_(false) {};

          virtual ~TCPListenerTimer() {
            stop();
            wait();
          }

          virtual void run() {
            while ( !stopped_ ) {
              {
                threads::Mutexer lock( listener_.now_mutex_ );
                gettimeofday( &(listener_.now_), NULL );
              }
              std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            }
          }

          /**
           * Stop the timer.
           */
          void stop() {
            stopped_ = true;
          }
        protected:
          /** The associated TCPListener. */
          TCPListener &listener_;
          /** True if stop() was called. */
          bool stopped_;
      };

    /** minservers YAML config name */
    const std::string yaml_minservers = "min-servers";
    /** maxservers YAML config name */
    const std::string yaml_maxservers = "max-servers";
    /** maxconnections YAML config name */
    const std::string yaml_maxconnections = "max-connections";
    /** maxqdepth YAML config name */
    const std::string yaml_maxqdepth = "max-queue-depth";
    /** sendbufsz YAML config name */
    const std::string yaml_sendbufsz = "send-buffer";
    /** recvbufsz YAML config name */
    const std::string yaml_recvbufsz = "receive-buffer";
    /** server_idle_ttl_s YAML config name */
    const std::string yaml_server_idle_ttl_s = "server-idle-ttl-s";
    /** pollbatch YAML config name */
    const std::string yaml_pollbatch = "poll-batch";
    /** listener_sleep_ms YAML config name */
    const std::string yaml_listener_sleep_ms = "listener-sleep-ms";
    /** throttle_sleep_us YAML config name */
    const std::string yaml_throttle_sleep_us = "throttle-sleep-us";
    /** cycle_max_throttles YAML config name */
    const std::string yaml_cycle_max_throttles = "cycle-max-throttles";
    /** stat_trc_interval_s YAML config name */
    const std::string yaml_stat_trc_interval_s = "stat-trc-interval-s";
    /** yaml_send_timeout_seconds YAML config name */
    const std::string yaml_send_timeout_seconds = "send-timeout-seconds";
    /** yaml_receive_timeout_seconds YAML config name */
    const std::string yaml_receive_timeout_seconds = "receive-timeout-seconds";



    TCPListener::Params::Params( const YAML::Node &node ) {
      minservers              = YAML_assign_by_key_with_default<size_t>( node, yaml_minservers, 2 );
      maxservers              = YAML_assign_by_key_with_default<size_t>( node, yaml_maxservers, 16 );
      maxconnections          = YAML_assign_by_key_with_default<size_t>( node, yaml_maxconnections, 500 );
      maxqdepth               = YAML_assign_by_key_with_default<size_t>( node, yaml_maxqdepth, 256 );
      sendbufsz               = YAML_assign_by_key_with_default<socklen_t>( node, yaml_sendbufsz, 16384 );
      recvbufsz               = YAML_assign_by_key_with_default<socklen_t>( node, yaml_recvbufsz, 32768 );
      server_idle_ttl_s       = YAML_assign_by_key_with_default<double>( node, yaml_server_idle_ttl_s, 300 );
      pollbatch               = YAML_assign_by_key_with_default<int>( node, yaml_pollbatch, 128 );
      listener_sleep_ms       = YAML_assign_by_key_with_default<int>( node, yaml_listener_sleep_ms, 200 );
      throttle_sleep_us       = YAML_assign_by_key_with_default<size_t>( node, yaml_throttle_sleep_us, 4000 );
      cycle_max_throttles     = YAML_assign_by_key_with_default<size_t>( node, yaml_cycle_max_throttles, 40 );
      stat_trc_interval_s     = YAML_assign_by_key_with_default<time_t>( node, yaml_stat_trc_interval_s, 300 );
      send_timeout_seconds    = YAML_assign_by_key_with_default<int>( node, yaml_send_timeout_seconds, 0 );
      receive_timeout_seconds = YAML_assign_by_key_with_default<int>( node, yaml_receive_timeout_seconds, 0 );
    }

    TCPListener::TCPListener( const Address& address, const Params &params ) :
      Thread() {
      construct( address, params );
    }

    void TCPListener::construct( const Address& address, const Params &params ) {
      listen_address_ = address;
      params_ = params;
      init_server_ = nullptr;
      stop_server_ = false;
      requests_q_sz_ = 0;

      read_event_mask_ = EPOLLIN | EPOLLPRI | EPOLLONESHOT | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLWAKEUP;
      //hangup_event_mask_ = EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLWAKEUP;
      hangup_event_mask_ = 0;
      if ( !listen_address_.isValid() ) throw_Exception( "invalid address" );
      listen_socket_ = Socket( false, SocketParams(
        listen_address_.getAddressFamily(),
        SocketParams::stSTREAM,
        SocketParams::pnHOPOPT )
      );
      listen_socket_.setReUseAddress();
      listen_socket_.setReUsePort();
      listen_socket_.setBlocking( false );
      listen_socket_.setSendBufSize( params_.sendbufsz );
      listen_socket_.setReceiveBufSize( params_.recvbufsz );
      listen_socket_.setSendTimeout( params_.send_timeout_seconds );
      listen_socket_.setReceiveTimeout( params_.receive_timeout_seconds );
      std::string proc_somaxconn = "/proc/sys/net/core/somaxconn";
      if ( !common::fileReadInt( proc_somaxconn, backlog_ ) )
        throw_Exception( "failed to read int from " << proc_somaxconn );

      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_minservers << " = " << params_.minservers );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_maxservers << " = " << params_.maxservers );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_maxconnections << " = " << params_.maxconnections );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_maxqdepth << " = " << params_.maxqdepth );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_sendbufsz << " = " << params_.sendbufsz );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_recvbufsz << " = " << params_.recvbufsz );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_server_idle_ttl_s << " = " << params_.server_idle_ttl_s );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_pollbatch << " = " << params_.pollbatch );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_listener_sleep_ms << " = " << params_.listener_sleep_ms );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_throttle_sleep_us << " = " << params_.throttle_sleep_us );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_cycle_max_throttles << " = " << params_.cycle_max_throttles );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_send_timeout_seconds << " = " << params_.send_timeout_seconds );
      Logger::getLogger()->statistics( common::Puts() <<
                                       yaml_receive_timeout_seconds << " = " << params_.receive_timeout_seconds );

      common::SystemError error = listen_socket_.listen( listen_address_, backlog_ );
      if ( error != common::SystemError::ecOK ) throw_SystemException( "listen failed", error );

      Logger::getLogger()->statistics( common::Puts() <<
                                       "address = " << listen_address_.asString(true) );
    }

    TCPListener::TCPListener( const YAML::Node &yaml ) {
      Params params( yaml );
      std::string saddr = YAML_assign_by_key<std::string>( yaml, "listen-address" );
      uint16_t port = YAML_assign_by_key<uint16_t>( yaml, "listen-port" );
      Address addr( saddr, port );
      if ( !addr.isValid() ) throw_Exception( "invalid listen address " << saddr << ":" << port );
      construct( addr, params );
    }

    bool TCPListener::waitForActivity( TCPServer* server ) {
      std::unique_lock<std::mutex> lk( mt_signal_ );
      return cv_signal_.wait_for( lk,
                                  std::chrono::milliseconds(params_.listener_sleep_ms),
                                  [this,server]{ return requests_q_sz_ || server->getRequestStop(); } );
    }

    void TCPListener::start( TCPServer *server ) {
      init_server_ = server;
      Thread::start();
    }

    void TCPListener::run() {
      struct epoll_event poll_sockets[params_.pollbatch];
      struct epoll_event set_event;
      event_maxconnections_reached_ = 0;
      int rc = 0;
      try {
        servers_.push_back(init_server_);
        memset(  poll_sockets, 0, sizeof(poll_sockets) );
        epoll_fd_ = epoll_create1( 0 );
        if ( epoll_fd_ < 0 )
          throw_SystemException( "TCPListener::run: epoll_create1 failed", errno );
        set_event.events = EPOLLIN | EPOLLPRI;
        set_event.data.fd = listen_socket_.getFD();
        rc = epoll_ctl( epoll_fd_, EPOLL_CTL_ADD, listen_socket_.getFD(), &set_event );
        if ( rc < 0 ) throw_SystemException( "TCPListener::run epoll_ctl failed", errno );

        Logger::getLogger()->debug( Puts() << "TCPListener::run starting " << params_.minservers << " servers" );
        init_server_->start();
        while ( servers_.size() < params_.minservers ) {
          TCPServer* add_server = init_server_->addServer();
          servers_.push_back(add_server);
          add_server->start();
        }

        gettimeofday( &prev_stat_time_, NULL );
        prev_stat_time_.tv_sec -= 1;
        gettimeofday( &stat_time_, NULL );
        gettimeofday( &warn_queue_time_, NULL );
        gettimeofday( &server_stopped_check_time_, NULL );
        gettimeofday( &now_, NULL );

        // start TCPListenerTimer
        TCPListenerTimer timer( *this );
        timer.start();
        do {
          logStats();
          addServers();
          throttle();
          // check for maxservers
          {
            threads::Mutexer lock( now_mutex_ );
            if ( servers_.size() == params_.maxservers &&
                 requests_q_sz_ > 2 * servers_.size() &&
                 now_.tv_sec > 30 + warn_queue_time_.tv_sec ) {
              Logger::getLogger()->warning( Puts() << "TCPListener::run queuing on maxservers #clients=" <<
                clients_.size() << " #servers=" << servers_.size() << " queue=" << requests_q_sz_ );
              gettimeofday( &warn_queue_time_, NULL );
            }
          }
          // wait for activity
          int rc = epoll_wait( epoll_fd_, poll_sockets, params_.pollbatch, params_.listener_sleep_ms );
          if ( rc < 0 && errno != EINTR )
            throw_SystemException( "TCPListener::run: epoll_wait failed", errno );
          else if ( rc > 0 ) {
            for ( int i = 0; i < rc; i++ ) {
              if ( poll_sockets[i].events != 0 ) {
                if ( poll_sockets[i].data.fd == listen_socket_.getFD() ) {
                  network::BaseSocket* client_sock;
                  do {
                    client_sock = listen_socket_.accept();
                    if ( !client_sock->isValid() ) break;
                    if ( clients_.size() >= params_.maxconnections ) {
                       client_sock->close();
                       delete client_sock;
                       event_maxconnections_reached_++;
                    } else {
                      {
                        threads::Mutexer lock( clientmutex_ );
                        clients_[client_sock->getFD()].pointer = client_sock;
                        clients_[client_sock->getFD()].state = SockState::New;
                      }
                      pushRequest( client_sock, SockState::New );
                      Logger::getLogger()->debug( Puts() << "TCPListener::run new client socket " <<
                                                  client_sock->debugString() << " from " <<
                                                  client_sock->getPeerAddress().asString() );
                      last_stats_.connections++;
                      client_sock->setBlocking( false );
                      client_sock->setTCPNoDelay( true );
                      client_sock->setSendTimeout( params_.send_timeout_seconds );
                      client_sock->setReceiveTimeout( params_.receive_timeout_seconds );
                      cv_signal_.notify_one();
                    }
                  } while ( client_sock->isValid() );
                } else if ( poll_sockets[i].data.fd >= 0 ) {
                  SockState combined_state = SockState::None;
                  if ( (poll_sockets[i].events & EPOLLIN) || (poll_sockets[i].events &  EPOLLPRI) ) {
                    Logger::getLogger()->debug( Puts() << "TCPListener::run EPOLLIN || EPOLLPRI on socket " <<
                                                poll_sockets[i].data.fd <<
                                                " events=(" << poll_sockets[i].events << ")" );
                    combined_state |= SockState::Read;
                    last_stats_.requests++;
                  }
                  if ( (poll_sockets[i].events & EPOLLRDHUP ) ||
                       (poll_sockets[i].events & EPOLLERR ) ||
                       (poll_sockets[i].events & EPOLLHUP ) ) {
                    combined_state |= SockState::Shut;
                    Logger::getLogger()->debug( common::Puts() <<
                                                "TCPListener::run hangup or error on socket " <<
                                                poll_sockets[i].data.fd <<
                                                " events=(" << poll_sockets[i].events << ")" );
                  }
                  if ( combined_state != SockState::None ) {
                    poll_sockets[i].events = 0;
                    pushRequest( poll_sockets[i].data.fd, combined_state );
                    cv_signal_.notify_one();
                  }
                } else {
                    Logger::getLogger()->warning( common::Puts() <<
                                                  "TCPListener::run epoll event on invalid descriptor " <<
                                                  poll_sockets[i].data.fd <<
                                                  " events=(" << poll_sockets[i].events << ")" );
                }
              }
            }
          }

          cleanStoppedServers();
        } while ( !stop_server_ );
      }
      catch( const dodo::common::Exception &e ) {
        cout << "dodo::common::Exception" << endl;
        Logger::getLogger()->error( common::Puts() << "TCPListener::run dodo::common::Exception " << e.what() );
      }
      catch( const std::exception &e ) {
        cout << "std::exception" << endl;
        Logger::getLogger()->error( common::Puts() << "TCPListener::run std::exception " << e.what() );
      }
      catch( ... ) {
        cout << "..." << endl;
        Logger::getLogger()->error( common::Puts() << "TCPListener::run unhandled exception" );
      }
      try {
        Logger::getLogger()->debug( common::Puts() << "TCPListener::run stop listener finish pending work" );
        while( requests_q_sz_  > 0 ) std::this_thread::sleep_for(20ms);
        Logger::getLogger()->debug( common::Puts() << "TCPListener::run stop TCPServers " );
        for ( auto srv : servers_ ) {
          srv->requestStop();
        }
        for ( auto srv : servers_ ) {
          srv->wait();
          delete srv;
        }
        Logger::getLogger()->debug( common::Puts() << "TCPListener::run stopped TCPServers " );
        servers_.clear();
        auto clients_copy = clients_;  // closeSocket modifies clients_
        for( auto c: clients_copy ) {
          Logger::getLogger()->trace( common::Puts() << "TCPListener::run close socket " << c.second.pointer );
          closeSocket( c.second.pointer );
        }
      }
      catch ( ... ) {
      }
    }

    void TCPListener::pushRequest( int fd, SockState state ) {
      {
        threads::Mutexer lock( clientmutex_ );
        clients_[fd].state |= state;
        requests_.push_back( &(clients_[fd]) );
        requests_q_sz_++;
        if ( ! (state & SockState::New) ) pollDel( clients_[fd].pointer );
      }
      Logger::getLogger()->debug( common::Puts() <<
                                  "TCPListener::pushRequest BaseSocket* socket " <<
                                  clients_[fd].pointer->debugString() << " state " << state );
    }

    void TCPListener::pushRequest( BaseSocket* socket, SockState state ) {
      {
        threads::Mutexer lock( clientmutex_ );
        clients_[socket->getFD()].state |= state;
        requests_.push_back( &(clients_[socket->getFD()]) );
        requests_q_sz_++;
        if ( ! (state & SockState::New) ) pollDel( clients_[socket->getFD()].pointer );
      }
      Logger::getLogger()->debug( common::Puts() <<
                                  "TCPListener::pushRequest BaseSocket* socket " << socket->debugString() <<
                                  " state " << state );
    }

    TCPListener::SockMap* TCPListener::popRequest() {
      SockMap* result = NULL;
      {
        threads::Mutexer lock( clientmutex_ );
        if ( requests_.size() > 0 ) {
          result = requests_.front();
          requests_.pop_front();
        }
      }
      if ( result )
        Logger::getLogger()->debug( common::Puts() <<
                                    "TCPListener::popRequest socket " << result->pointer->debugString() <<
                                    " state " << result->state  << " sockmapstate=" << clients_[result->pointer->getFD()].state );
      return result;
    }

    void TCPListener::releaseRequest( BaseSocket* socket, SockState state ) {
      Logger::getLogger()->debug( common::Puts() <<
                                  "TCPListener::releaseRequest socket " << socket->debugString() <<
                                  " state " << state  << " sockmapstate=" << clients_[socket->getFD()].state );
      requests_q_sz_--;
      if ( state & TCPListener::SockState::Shut ) {
        closeSocket( socket );
      } else {
        threads::Mutexer lock( clientmutex_ );
        pollAdd( socket, read_event_mask_ | hangup_event_mask_ );
        clients_[socket->getFD()].state ^= state;
      }
    }

    void TCPListener::closeSocket( BaseSocket *s ) {
      int fd = -1;
      {
        threads::Mutexer lock( clientmutex_ );
        fd = s->getFD();
        clients_.erase( fd );
        s->close();
        delete s;
      }
      Logger::getLogger()->debug( common::Puts() << "TCPListener::closeSocket socket " << s << " fd=" << fd );
    }

    void TCPListener::pollAdd( const BaseSocket* s, uint32_t events ) {
      struct epoll_event set_event;
      set_event.events = events;
      set_event.data.fd = s->getFD();
      int rc = epoll_ctl( epoll_fd_, EPOLL_CTL_ADD, s->getFD(), &set_event );
      if ( rc < 0 ) throw_SystemException( "TCPListener::pollAdd epoll_ctl failed socket " << s->debugString(), errno );
      Logger::getLogger()->debug( common::Puts() << "TCPListener::pollAdd socket " << set_event.data.fd << " events=" << events );
    }

    void TCPListener::pollMod( const BaseSocket* s, uint32_t events ) {
      struct epoll_event set_event;
      set_event.events = events;
      set_event.data.fd = s->getFD();
      int rc = epoll_ctl( epoll_fd_, EPOLL_CTL_MOD, s->getFD(), &set_event );
      if ( rc < 0 ) throw_SystemException( "TCPListener::pollMod epoll_ctl failed socket " << s->debugString(), errno );
      Logger::getLogger()->debug( common::Puts() << "TCPListener::pollMod socket " << set_event.data.fd << " events=" << events );
    }

    void TCPListener::pollDel( const BaseSocket* s ) {
      struct epoll_event set_event;
      set_event.events = 0;
      set_event.data.fd = s->getFD();
      int rc = epoll_ctl( epoll_fd_, EPOLL_CTL_DEL, s->getFD(), &set_event );
      if ( rc < 0 ) throw_SystemException( "TCPListener::pollDel epoll_ctl failed socket " << s->debugString(), errno );
      Logger::getLogger()->debug( common::Puts() << "TCPListener::pollDel socket " << set_event.data.fd );
    }

    void TCPListener::addServers() {
      while ( requests_q_sz_ > servers_.size() && servers_.size() < params_.maxservers  ) {
        TCPServer* add_server = init_server_->addServer();
        servers_.push_back(add_server);
        add_server->start();
        Logger::getLogger()->debug( common::Puts() <<
                                    "TCPListener::addServers +1" );
      }
    }

    void TCPListener::throttle() {
      if ( requests_q_sz_ > params_.maxqdepth ) {
        unsigned long last_pending =  requests_q_sz_;
        unsigned int c = 0;
        while ( requests_q_sz_ >= last_pending && c++ < params_.cycle_max_throttles ) {
          std::this_thread::sleep_for( std::chrono::microseconds( params_.throttle_sleep_us ) );
          last_stats_.throttles++;
        }
      }
    }

    void TCPListener::logStats() {
      struct timeval l_now;
      {
        threads::Mutexer lock( now_mutex_ );
        l_now = now_;
      }
      if ( l_now.tv_sec > params_.stat_trc_interval_s + stat_time_.tv_sec ) {
        snapRUsage();
        if ( event_maxconnections_reached_ ) {
          Logger::getLogger()->warning( common::Puts() <<
                                        "TCPListener maxconnections=" << params_.maxconnections <<
                                        " limited " << event_maxconnections_reached_ << " times since last stats" );
          event_maxconnections_reached_ = 0;
        }
        Logger::getLogger()->statistics( common::Puts() <<
                                         "TCPListener #clients=" <<
                                         clients_.size() << " #servers=" << servers_.size() << " #queued=" << requests_q_sz_ <<
                                         " conn=" << (double)( last_stats_.connections - prev_stats_.connections ) /
                                         getSecondDiff(prev_stat_time_,stat_time_ ) <<
                                         "/s req=" << (double)( last_stats_.requests - prev_stats_.requests ) /
                                         getSecondDiff(prev_stat_time_,stat_time_) <<
                                         "/s throttle=" << (double)( last_stats_.throttles - prev_stats_.throttles ) /
                                         getSecondDiff(prev_stat_time_,stat_time_) << "/s" );
        Logger::getLogger()->statistics( common::Puts() <<
                                         common::Puts::setprecision(2) <<
                                         "TCPListener ucpu=" << getLastUserCPU() <<
                                         " scpu=" << getLastSysCPU() <<
                                         " minflt=" << getLastMinFltRate() << "/s" <<
                                         " majflt=" << getLastMajFltRate() << "/s" <<
                                         " bi=" << getLastBlkInRate() << "/s" <<
                                         " bo=" << getLastBlkOutRate() << "/s" <<
                                         " yield=" << getLastVCtx() << "/s" <<
                                         " ctxsw=" << getLastICtx() << "/s" <<
                                         " maxrss=" << getMaxRSS() );
        for ( auto srv : servers_ ) {
         Logger::getLogger()->statistics( common::Puts() <<
                                          common::Puts::setprecision(2) <<
                                          "TCPServer id " << srv->getTID() <<
                                          (srv->isBusy()?" busy-":" idle-") << srv->getState() <<
                                          " ucpu=" << srv->getLastUserCPU() <<
                                          " scpu=" << srv->getLastSysCPU() <<
                                          " minflt=" << srv->getLastMinFltRate() << "/s" <<
                                          " majflt=" << srv->getLastMajFltRate() << "/s" <<
                                          " bi=" << srv->getLastBlkInRate() << "/s" <<
                                          " bo=" << srv->getLastBlkOutRate() << "/s" <<
                                          " yield=" << srv->getLastVCtx() << "/s" <<
                                          " ctxsw=" << srv->getLastICtx() << "/s" );
        }
        prev_stat_time_ = stat_time_;
        prev_stats_ = last_stats_;
        gettimeofday( &stat_time_, NULL );
      }
    }

    void TCPListener::cleanStoppedServers() {
      struct timeval l_now;
      {
        threads::Mutexer lock( now_mutex_ );
        l_now = now_;
      }
      if ( server_stopped_check_time_.tv_sec < l_now.tv_sec - 4 ) {
        gettimeofday( &server_stopped_check_time_, NULL );
        size_t num_stopped = 0;
        std::list<TCPServer*>::iterator i_server = servers_.begin();
        while ( i_server != servers_.end() ) {
          if ( *i_server != init_server_ ) {
            if ( (*i_server)->hasStopped() ) {
              std::thread::id tid = (*i_server)->getId();
              (*i_server)->wait();
              TCPServer* thisserver = (*i_server);
              i_server = servers_.erase(i_server);
              delete thisserver;
              Logger::getLogger()->debug( common::Puts() <<
                                          "TCPListener::run deleted stopped TCPServer " << common::Puts::hex() <<
                                          tid << common::Puts::dec() );
            } else {
              if ( num_stopped == 0 &&
                   servers_.size() > params_.minservers &&
                   (*i_server)->getIdleSeconds() > params_.server_idle_ttl_s &&
                   !(*i_server)->isBusy() ) {
                  (*i_server)->requestStop();
                  cv_signal_.notify_all();
                  num_stopped--;
                  Logger::getLogger()->debug( common::Puts() <<
                                              "TCPListener::run stop idle TCPServer " << common::Puts::hex() <<
                                              (*i_server)->getId() << common::Puts::dec() );
              }
              i_server++;
            }
          } else {
            if ( init_server_->hasStopped() ) {
              init_server_->wait();
              i_server = servers_.erase(i_server);
              TCPServer* new_server = init_server_->addServer();
              new_server->start();
              delete init_server_;
              init_server_ = new_server;
              servers_.push_back( init_server_ );
              Logger::getLogger()->debug( common::Puts() <<
                                          "TCPListener::run replaced stopped init TCPServer " << common::Puts::hex() <<
                                          init_server_->getTID() << common::Puts::dec() );
            }
            i_server++;
          }
        }
      }
    }

  }

}
