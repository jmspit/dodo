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
 * @file uri.hpp
 * Defines the dodo::network::URI class.
 */

#ifndef dodo_network_uri_hpp
#define dodo_network_uri_hpp

#include <string>

namespace dodo {

  namespace network {

    /**
     * Uniform Resource Identifier.
     * @see https://en.wikipedia.org/wiki/Uniform_Resource_Identifier
     */
    class URI {
      public:

        URI() { reset(); };

        URI( const std::string &scheme,
             const std::string &userinfo,
             const std::string &host,
             const std::string &port,
             const std::string &path,
             const std::string &query,
             const std::string &fragment ) :
             scheme_(scheme),
             userinfo_(userinfo),
             host_(host),
             port_(port),
             path_(path),
             query_(query),
             fragment_(fragment) {};

        /**
         * Copy constructor.
         * @param uri The URI to copy.
         */
        URI( const URI &uri ) { *this = uri; };

        /**
         * Desctructor.
         */
        ~URI() {};

        /**
         * Parse the string as an URI. Rteurn false and set idxfail to the failing char
         * if the pasre fails, true if the pasre succeeds.
         * @param s The string to parse
         * @param idxfail If parse fails, the index of the failing character
         * @return true if the pasre was successful.
         */
        bool parse( const std::string &s, size_t &idxfail );

        /**
         * Return the URI as a std::string.
         * @return the std::string representation of the URI.
         */
        std::string asString() const;

        /**
         * Return the URI scheme std::string.
         * @return the scheme of the URI.
         */
        std::string getScheme() const { return scheme_; };

        /**
         * Set the URI scheme.
         * @param scheme The scheme to set.
         */
        void setScheme( const std::string &scheme ) { scheme_ = scheme; };

        /**
         * Return the user-info std::string.
         * @return the user-info of the URI.
         */
        std::string getUserInfo() const { return userinfo_; };

        /**
         * Set the URI user-info.
         * @param userinfo The user-info to set.
         */
        void setUserInfo( const std::string &userinfo ) { userinfo_ = userinfo; };

        /**
         * Return the host std::string.
         * @return the host of the URI.
         */
        std::string getHost() const { return host_; };

        /**
         * Set the URI host.
         * @param host The host to set.
         */
        void setHost( const std::string &host ) { host_ = host; };

        /**
         * Return the port std::string.
         * @return the port of the URI.
         */
        std::string getPort() const { return port_; };

        /**
         * Set the URI port.
         * @param port The port to set.
         */
        void setPort( const std::string &port ) { port_ = port; };

        /**
         * Return the path std::string.
         * @return the path of the URI.
         */
        std::string getPath() const { return path_; };

        /**
         * Set the URI path.
         * @param path The path to set.
         */
        void setPath( const std::string &path ) { path_ = path; };

        /**
         * Return the query std::string.
         * @return the query of the URI.
         */
        std::string getQuery() const { return query_; };

        /**
         * Set the URI query.
         * @param query The query to set.
         */
        void setQuery( const std::string &query ) { query_ = query; };

        /**
         * Return the fragment std::string.
         * @return the fragment of the URI.
         */
        std::string getFragment() const { return fragment_; };

        /**
         * Set the URI fragment.
         * @param fragment The fragment to set.
         */
        void setFragment( const std::string &fragment ) { fragment_ = fragment; };

        /**
         * Assignment operator.
         * @param uri The URI to assign.
         * @return a reference to this URI.
         */
        URI& operator=( const URI& uri ) {
          scheme_ = uri.scheme_;
          userinfo_ = uri.userinfo_;
          host_ = uri.host_;
          port_ = uri.port_;
          path_ = uri.path_;
          query_ = uri.query_;
          fragment_ = uri.fragment_;
          return *this;
        }

        /**
         * Equality operator.
         * @param uri The URI to compare to.
         * @return true if the URIs are equal.
         */
        bool operator==( const URI& uri ) {
          return scheme_ == uri.scheme_ &&
                 userinfo_ == uri.userinfo_ &&
                 host_ == uri.host_ &&
                 port_ == uri.port_ &&
                 path_ == uri.path_ &&
                 query_ == uri.query_ &&
                 fragment_ == uri.fragment_;
        }

        /**
         * Inequality operator.
         * @param uri The URI to compare to.
         * @return true if the URIs are unequal.
         */
        bool operator!=( const URI& uri ) {
          return scheme_ != uri.scheme_ ||
                 userinfo_ != uri.userinfo_ ||
                 host_ != uri.host_ ||
                 port_ != uri.port_ ||
                 path_ != uri.path_ ||
                 query_ != uri.query_ ||
                 fragment_ != uri.fragment_;
        }

        /**
         * Ordering operator.
         * @param uri The URI to compare to.
         * @return true if this URI has lower order.
         */
        bool operator<( const URI& uri ) {
          return scheme_ < uri.scheme_ ||
                 ( scheme_ == uri.scheme_ && userinfo_ < uri.userinfo_ ) ||
                 ( scheme_ == uri.scheme_ && userinfo_ == uri.userinfo_ && host_ < uri.host_  ) ||
                 ( scheme_ == uri.scheme_ && userinfo_ == uri.userinfo_ && host_ == uri.host_ && port_ < uri.port_ ) ||
                 ( scheme_ == uri.scheme_ && userinfo_ == uri.userinfo_ && host_ == uri.host_ && port_ == uri.port_ && path_ < uri.path_ ) ||
                 ( scheme_ == uri.scheme_ && userinfo_ == uri.userinfo_ && host_ == uri.host_ && port_ == uri.port_ && path_ == uri.path_ && query_ < uri.query_ ) ||
                 ( scheme_ == uri.scheme_ && userinfo_ == uri.userinfo_ && host_ == uri.host_ && port_ == uri.port_ && path_ == uri.path_ && query_ == uri.query_ && fragment_ < uri.fragment_ );
        }

      private:

        /** States for the parsingg automaton. */
        enum ParseState {
          psSchemeStart,
          psSchemeEnd,
          psAuthorityStart,
          psUserInfoEnd,
          psHostStart,
          psHostEnd,
          psPortStart,
          psPortEnd,
          psPathStart,
          psPathEnd,
          psQueryStart,
          psQueryEnd,
          psFragmentStart,
          psTCP6Start,
          psPCTEncoded,

          psError,
          psDone
        };

        /**
         * Check if c is a valid Octet
         * @param c The char to check.
         * @return true if the char is ok.
         */
        bool verifyOctetChar( char c );

        /**
         * Check if c is a valid Scheme char
         * @param c The char to check.
         * @return true if the char is ok.
         */
        bool verifySchemeChar( char c );

        /**
         * Check if c is a valid UserInfo or host char
         * @param c The char to check.
         * @return true if the char is ok.
         */
        bool verifyUserInfoHostChar( char c );

        /**
         * Check if c is a valid Path char
         * @param c The char to check.
         * @return true if the char is ok.
         */
        bool verifyPathChar( char c );

        /**
         * Check if c is a valid Fragment char
         * @param c The char to check.
         * @return true if the char is ok.
         */
        bool verifyQueryFragmentChar( char c );

        /**
         * Check if c is a valid TCP6 address char
         * @param c The char to check.
         * @return true if the char is ok.
         */
        bool verifyTCP6Char( char c );

        /**
         * Reste all data to empty strings.
         */
        void reset();

        /** The scheme */
        std::string scheme_;
        /** The userinfo */
        std::string userinfo_;
        /** The host */
        std::string host_;
        /** The port */
        std::string port_;
        /** The path */
        std::string path_;
        /** The query */
        std::string query_;
        /** The fragment */
        std::string fragment_;
    };

  }

}

#endif
