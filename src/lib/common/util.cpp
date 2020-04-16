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
 * @file util.cpp
 * Implement utility things.
 */

#include "common/util.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <ios>

namespace dodo::common {

  bool fileReadInt( const std::string &file, int &i ) {
    std::ifstream f(file);
    f >> i;
    return f.good();
  }

  std::string strASCII( char c ) {
    if ( c < 32 || c == 127 ) {
      switch ( c ) {
        case 0: return "NUL";
        case 1: return "SOH";
        case 2: return "STX";
        case 3: return "ETX";
        case 4: return "EOT";
        case 5: return "ENQ";
        case 6: return "ACK";
        case 7: return "BEL";
        case 8: return " BS";
        case 9: return " HT";
        case 10: return " LF";
        case 11: return " VT";
        case 12: return " FF";
        case 13: return " CR";
        case 14: return " SO";
        case 15: return " SI";
        case 16: return "DLE";
        case 17: return "DC1";
        case 18: return "DC2";
        case 19: return "DC3";
        case 20: return "DC4";
        case 21: return "NAK";
        case 22: return "SYN";
        case 23: return "ETB";
        case 24: return "CAN";
        case 25: return " EM";
        case 26: return "SUB";
        case 27: return "ESC";
        case 28: return " FS";
        case 29: return " GS";
        case 30: return " RS";
        case 31: return " US";
        case 127: return "DEL";
        default: return "???";
      }
    } else if ( c < 255 ) {
      std::stringstream ss;
      ss << std::setfill(' ') << std::setw(3) << c;
      return ss.str();
    } else {
      std::stringstream ss;
      ss << std::setw(3) << std::setfill('0') << std::hex << std::setw(3) << c;
      return ss.str();
    }
  }

  void dumpBinaryData( std::ostream &out, const std::string &s, size_t width ) {
    std::ios_base::fmtflags orgflags = out.flags();
    size_t idx = 0;
    size_t line_idx = 0;
    std::stringstream binary_line;
    std::stringstream char_line;
    while ( idx < s.size() ) {
      if ( line_idx  >= width ) {
        out << std::setfill('0') << std::setw(6) << idx-width << binary_line.str() << std::endl;
        out << std::string(6,' ') << char_line.str() << std::endl;
        binary_line.str( "" );
        char_line.str( "" );
        line_idx = 0;
      }
      unsigned int v = (unsigned char)s[idx];
      binary_line << ' ' << std::setw(2) << std::setfill('0') << std::hex << v << ' ';
      char_line   << strASCII( s[idx] ) << ' ';
      idx++;
      line_idx++;
    }
    if ( line_idx ) {
      out << std::setfill('0') << std::setw(6) << idx-width << binary_line.str() << std::endl;
      out << std::string(6,' ') << char_line.str();
    }
    out.flags( orgflags );
  }

}
