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
 * @file uri.cpp
 * Implements the dodo::network::URI class.
 */

#include "network/uri.hpp"

#include <iostream>
#include <sstream>

namespace dodo {

  namespace network {


    void URI::reset() {
      scheme_   = "";
      userinfo_ = "";
      host_     = "";
      port_     = "";
      path_     = "";
      query_    = "";
      fragment_ = "";
    }

    std::string URI::asString() const {
      std::stringstream ss;
      ss << scheme_ << ":";
      if ( host_.size() ) ss << "//";
      if ( userinfo_.size() ) ss << userinfo_ << '@';
      if ( host_.size() ) ss << host_;
      if ( port_.size() ) ss << ':' << port_;
      ss << path_;
      if ( query_.size() ) ss << '?' << query_;
      if ( fragment_.size() ) ss << '#' << fragment_;
      return ss.str();
    }

    bool URI::verifySchemeChar( char c ) {
      return std::isalpha(c) ||
             std::isdigit(c) ||
             c == '+' ||
             c == '.' ||
             c == '-';
    }

    bool URI::verifyOctetChar( char c ) {
      return std::isalpha(c) ||
             std::isdigit(c) ||
             c == '-' ||
             c == '.' ||
             c == '_' ||
             c == '~';
    }

    bool URI::verifyTCP6Char( char c ) {
      return ( ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) ||
             std::isdigit(c) ||
             c == ':';
    }

    bool URI::verifyUserInfoHostChar( char c ) {
      return verifyOctetChar(c) ||
             c == '!' ||
             c == '$' ||
             c == '&' ||
             c == '\'' ||
             c == '(' ||
             c == ')' ||
             c == '*' ||
             c == '+' ||
             c == ',' ||
             c == ';' ||
             c == '=';
    }

    bool URI::verifyPathChar( char c ) {
      return verifyUserInfoHostChar(c) ||
             c == ':' ||
             c == '/' ||
             c == '@';
    }

    bool URI::verifyQueryFragmentChar( char c ) {
      return verifyUserInfoHostChar(c) ||
             c == ':' ||
             c == '@' ||
             c == '/' ||
             c == '?';
    }

    bool URI::parse( const std::string &s, size_t &idx ) {
      //std::cout << s << std::endl;
      uint16_t prev_state, return_state, state;
      idx = 0;
      size_t mark = 0;
      reset();
      prev_state = psError;
      return_state = psError;
      state = psSchemeStart;
      while ( state != psDone && state != psError && idx < s.size()  ) {
        //std::cout << idx << " " << s[idx] << " " << s[mark] << " " << state << std::endl;
        switch ( state ) {

          case psSchemeStart:
            if ( prev_state != psSchemeStart && !std::isalpha(s[idx] ) )  state = psError;
            else if ( ( s[idx] ) == ':' ) state = psSchemeEnd; else {
              if ( !verifySchemeChar( s[idx] ) ) state = psError; else idx++;
            }
            break;

          case psSchemeEnd:
            scheme_ = s.substr(mark,idx-mark);
            //std::cout << "scheme=" << scheme_ << std::endl;
            idx++;
            if ( s[idx] == '/' ) {
              idx++;
              if ( s[idx] == '/' ) {
                mark = ++idx;
                state = psAuthorityStart;
              } else state = psError;
            } else {
              state = psPathStart;
              mark = idx;
            }
            break;

          case psAuthorityStart:
            if ( s[idx] == '@' ) {
              state = psUserInfoEnd;
            } else if ( s[idx] == '/' ) {
              state = psHostEnd;
            } else if ( s[idx] == ':' ) {
              state = psHostEnd;
            } else if ( s[idx] == '[' ) {
              state = psTCP6Start;
              idx++;
            } else if ( s[idx] == '%' ) {
              return_state = state;
              state = psPCTEncoded;
            } else {
              if ( verifyUserInfoHostChar( s[idx] ) ) {
                idx++;
              } else state = psError;
            }
            break;

          case psUserInfoEnd:
            if ( idx > mark ) {
              userinfo_ = s.substr(mark,idx-mark);
              //std::cout << "userinfo=" << userinfo_ << std::endl;
              mark = ++idx;
              state = psHostStart;
            } else state = psError;
            break;

          case psHostStart:
            if ( s[idx] == '[' ) {
              state = psTCP6Start;
              idx++;
            }
            else if ( s[idx] == ':' || s[idx] == '/' ) state = psHostEnd;
            else if ( verifyUserInfoHostChar( s[idx] ) ) idx++;
            else if ( s[idx] == '%' ) {
              return_state = state;
              state = psPCTEncoded;
            }
            else state = psError;
            break;

          case psTCP6Start:
            if ( s[idx] == ']' ) {
              state = psHostEnd;
              idx++;
            }
            else if ( verifyTCP6Char( s[idx] ) ) idx++;
            else state = psError;
            break;

          case psHostEnd:
            if ( idx > mark ) {
              host_ = s.substr(mark,idx-mark);
              //std::cout << "host=" << host_ << std::endl;
              if ( s[idx] == ':' ) {
                state = psPortStart;
                mark = ++idx;
              } else {
                state = psPathStart;
                mark = idx;
              }
            } else state = psError;
            break;

          case psPortStart:
            if ( ! std::isdigit( s[idx] ) ) state = psPortEnd; else idx++;
            break;

          case psPortEnd:
            if ( idx > mark ) {
              port_ = s.substr(mark,idx-mark);
              //std::cout << "port=" << port_ << std::endl;
              state = psPathStart;
              mark = idx;
            } else state = psError;
            break;

          case psPathStart:
            if ( s[idx] == '?' ) state = psPathEnd;
              else if ( s[idx] == '#' ) state = psPathEnd;
              else if ( verifyPathChar( s[idx] ) ) idx++;
              else if ( s[idx] == '%' ) {
                return_state = state;
                state = psPCTEncoded;
              } else state = psError;
            break;

          case psPathEnd:
            if ( idx > mark ) {
              path_ = s.substr(mark,idx-mark);
              //std::cout << "path=" << path_ << std::endl;
              if ( s[idx] == '?' ) state = psQueryStart;
                else if ( s[idx] == '#' ) state = psFragmentStart;
              mark = ++idx;
            } else state = psError;
            break;

          case psQueryStart:
            if ( s[idx] == '#' )
              state = psQueryEnd;
              else if ( verifyQueryFragmentChar( s[idx] ) ) idx++;
              else if ( s[idx] == '%' ) {
                return_state = state;
                state = psPCTEncoded;
              }
              else state = psError;
            break;

          case psQueryEnd:
            if ( idx > mark ) {
              query_ = s.substr(mark,idx-mark);
              //std::cout << "query=" << query_ << std::endl;
              state = psFragmentStart;
              mark = ++idx;
            } else state = psError;
            break;

          case psFragmentStart:
            if ( verifyQueryFragmentChar( s[idx] ) ) idx++;
            else if ( s[idx] == '%' ) {
              return_state = state;
              state = psPCTEncoded;
            }
            else state = psError;
            break;

          case psPCTEncoded:
            if ( s[idx] == '%' ) {
              idx++;
              if ( idx < s.size() && std::isxdigit( s[idx] ) ) {
                idx++;
                if ( idx < s.size() && std::isxdigit( s[idx] ) ) {
                  idx++;
                  state = return_state;
                } else state = psError;
              } else state = psError;
            } else state = psError;
            break;

          default:
            idx++;

        }
        prev_state = state;
      }
      switch ( state ) {
        case psAuthorityStart:
          host_ = s.substr(mark,s.size()-mark);
          //std::cout << "host=" << host_ << std::endl;
          break;
        case psPathStart:
          path_ = s.substr(mark,s.size()-mark);
          //std::cout << "path=" << path_ << std::endl;
          break;
        case psQueryStart:
          query_ = s.substr(mark,s.size()-mark);
          //std::cout << "query=" << query_ << std::endl;
          break;
        case psFragmentStart:
          fragment_ = s.substr(mark,s.size()-mark);
          //std::cout << "fragment=" << fragment_ << std::endl;
          break;
      }
      return state != psError;
    }

  }

}

