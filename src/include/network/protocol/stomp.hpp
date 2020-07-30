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
 * @file stomp.hpp
 * Defines the dodo::network::protocol::Stomp classes.
 */

#ifndef network_protocol_stomp_hpp
#define network_protocol_stomp_hpp

#include "common/octetarray.hpp"

#include <list>
#include <string>

namespace dodo::network::protocol::stomp {

  /**
   * A generic STOMP 1.2 frame.
   */
  class Frame {
    public:

      /** EOL sequence - use only \n as the protocl allows it and the extra \n serves no purpose. */
      const common::OctetArray eol{ "\n" };

      /** CONNECT command. */
      const common::OctetArray command_connect{ "STOMP" };

      /** CONNECTED command. */
      const common::OctetArray command_connected{ "CONNECTED" };

      /** STOMP 1.2 accept header. */
      const common::OctetArray header_accept_version_1_2{ "accept-version:1.2" };

      /**
       * The manner in which frames match the STOMP protocol.
       */
      enum class FrameMatch {
        NoMatch,          /**< Mismatch */
        IncompleteMatch,  /**< Match, but incomplete */
        FullMatch         /**< Complete match */
      };

      /**
       * STOMP versions. Only the latest 1.2 version is supported.
       */
      enum class Version {
        v1_2 /**< STOMP 1.2 */
      };

      /**
       * Checks how the data matches a frame specification.
       * @param frame The frame / common::OctetArray to match against.
       * @param errors If returning FrameMatch::NoMatch, one or more errors.
       * @return FrameMatch::IncompleteMatch if the frame matches but is incomplete,
       *         FrameMatch::FullMatch if it matches completely.
       */
      virtual FrameMatch match( const common::OctetArray& frame, std::list<std::string> &errors ) const = 0;

      /**
       * Generate a frame.
       * @param frame The generation destination, which is overwritten.
       * @return void
       */
      virtual void generate( common::OctetArray& frame ) const = 0;

    protected:

      /**
       * Read a command from a frame.
       * @param frame The (incomplete) frame
       * @param index The index into fame (0 for commands).
       * @param command The command to read.
       * @return FrameMatch::IncompleteMatch if the frame matches but is incomplete,
       *         FrameMatch::FullMatch if it matches completely.
       */
      FrameMatch readCommand( const common::OctetArray& frame, size_t &index, const common::OctetArray& command ) const;

  };

  /**
   * A STOMP CONNECT frame.
   */
  class Connect : public Frame {
    public:

      /**
       * Constructor.
       * @param version The STOMP protocol version.
       */
      Connect( const Version &version = Version::v1_2 ) : Frame(),
        host_(""),
        login_(""),
        passcode_(""),
        version_(version),
        heartbeat_out_ms_(0),
        heartbeat_in_ms_(0)  {}


      /**
       * Checks how the data matches a frame specification.
       * @param frame The frame / common::OctetArray to match against.
       * @param errors If returning FrameMatch::NoMatch, one or more errors.
       * @return FrameMatch::IncompleteMatch if the frame matches but is incomplete,
       *         FrameMatch::FullMatch if it matches completely.
       */
      virtual FrameMatch match( const common::OctetArray& frame, std::list<std::string> &errors ) const;

      /**
       * Generate a STOMP (CONNECT) frame.
       * @param frame The generation destination, which is overwritten.
       * @return void
       */
      virtual void generate( common::OctetArray& frame ) const;

      /**
       * Return the STOMP host.
       * @return The host.
       */
      std::string getHost() const { return host_; };

      /**
       * Set the STOMP host.
       * @param host The host to set.
       */
      void setHost( const std::string &host ) { host_ = host; };

      /**
       * Return the login.
       * @return The login.
       */
      std::string getLogin() const { return login_; };

      /**
       * Set the login.
       * @param login The login to set.
       */
      void setLogin( const std::string &login ) { login_ = login; };

      /**
       * Return the passcode.
       * @return The passcode.
       */
      std::string getPasscode() const { return passcode_; };

      /**
       * Set the login.
       * @param passcode The passcode to set.
       */
      void setPasscode( const std::string &passcode ) { passcode_ = passcode; }


      /**
       * Return the outgoing heartbeat in milliseconds.
       * @return the outgoing heartbeat interval in milliseconds.
       */
      size_t getHeartbeatOut() const { return heartbeat_out_ms_; }

      /**
       * Set the heartbeat.
       * @param out outgoing heartbeat interval in milliseconds.
       * @param in incoming heartbeat interval in milliseconds.
       */
      void setHeartbeat( size_t out, size_t in ) { heartbeat_out_ms_ = out; heartbeat_in_ms_ = in; }

    protected:
      /** STOMP host to CONNECT. */
      std::string host_;

      /** Optional login. */
      std::string login_;

      /** Optional passcode (required if login is not empty) */
      std::string passcode_;

      /** STOMP protocol version required in CONNECT handshake. */
      Version version_;

      /** Offered outgoing heartbeat delay in milliseconds. */
      size_t heartbeat_out_ms_;

      /** Desired incoming heartbeat delay in milliseconds. */
      size_t heartbeat_in_ms_;
  };

  /**
   * A STOMP CONNECTED frame.
   */
  class Connected : public Frame {
    public:

      /**
       * Constructor.
       * @param version The STOMP protocol version.
       */
      Connected( const Version &version ) :
        version_(version),
        heartbeat_out_ms_(0),
        heartbeat_in_ms_(0)
        {}

      /**
       * Generate a CONNECTED frame.
       * @param frame The generation destination, which is overwritten.
       * @return void
       */
      virtual void generate( common::OctetArray& frame ) const;

    protected:

      /** STOMP protocol version required in CONNECT handshake. */
      Version version_;

      /** Offered outgoing heartbeat delay in milliseconds. */
      size_t heartbeat_out_ms_;

      /** Desired incoming heartbeat delay in milliseconds. */
      size_t heartbeat_in_ms_;

      /** The session id. */
      std::string session_id_;

      /** The server id. */
      std::string server_;


  };

}

#endif