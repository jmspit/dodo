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
 * @file httpversion.hpp
 * Defines the dodo::network::protocol::http::HTTPVersion class.
 */

#ifndef dodo_network_protocol_http_httpversion_hpp
#define dodo_network_protocol_http_httpversion_hpp

#include <network/protocol/http/httpfragment.hpp>
#include <string>

namespace dodo {

  namespace network {

    /**
     * HHTP version comprising a major and minor number,
     * convertable from and to string as in HTTP requests.
     */
    class HTTPVersion : public HTTPFragment {
      public:
        /**
         * Default constructor to HTTP/1.0
         */
        HTTPVersion() : major_(1), minor_(0) {};

        /**
         * Construct from major and minor.
         * @param major the major to set.
         * @param minor the minor to set.
         */
        explicit HTTPVersion( unsigned int major, unsigned int minor ) : major_(major), minor_(minor) {};

        /**
         * Convert to string as 'HTTP/major_.minor_'
         * @return the HTTP VERSION string.
         */
        virtual std::string asString() const {
          std::stringstream ss;
          ss << std::fixed << "HTTP/" << major_ << "." << minor_;
          return ss.str();
        }

        /**
         * Construct from current position in the VirtualReadBuffer 'HTTP/major_.minor_'.
         * @param buffer the VirtualReadBuffer to read from.
         * @return the HTTPFragment::ParseError, peOk if no error.
         */
        virtual ParseResult parse( VirtualReadBuffer& buffer );

        /**
         * Return the major version.
         * @return The major version.
         */
        unsigned int getMajor() const { return major_; };

        /**
         * Return the minor version.
         * @return The minor version.
         */
        unsigned int getMinor() const { return minor_; };

        /**
         * Assignment operator.
         * @param version The version to assigne.
         * @return This HTTPVersion.
         */
        HTTPVersion& operator=( const HTTPVersion& version ) { major_ = version.major_; minor_ = version.minor_; return *this; };

        /**
         * Equality operator.
         * @param version The HTTPVersion to compare to.
         * @return True when equal.
         */
        bool operator==( const HTTPVersion& version ) const { return major_ == version.major_ && minor_ == version.minor_; };

        /**
         * Less-than operator.
         * @param version The HTTPVersion to compare to.
         * @return True if this < version.
         */
        bool operator<( const HTTPVersion& version ) const { return major_ < version.major_ ||
                                                                    (major_ == version.major_ && minor_ < version.minor_); };

        /**
         * Greater than operator.
         * @param version The HTTPVersion to compare to.
         * @return True if this > version.
         */
        bool operator>( const HTTPVersion& version ) const { return major_ > version.major_ ||
                                                                    (major_ == version.major_ && minor_ > version.minor_); };

      private:
        /** The major version. */
        unsigned int major_;
        /** The minor version. */
        unsigned int minor_;
    };

  }

}

#endif