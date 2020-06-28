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

#include "common/exception.hpp"
#include "common/util.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <ios>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


namespace dodo::common {

  bool fileReadInt( const std::string &file, int &i ) {
    std::ifstream f(file);
    f >> i;
    return f.good();
  }

  /**
   * Return ASCII representation of char.
   * @param c The char.
   * @return the ASCII representation.
   */
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

  /**
   * Dump binary data to a std::ostream.
   * @param out The std::stream to dump to.
   * @param s The char stream.
   * @param width The max character width of the output.
   */
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

  std::string bio2String( BIO* bio ) {
    char *data = NULL;
    long length = BIO_get_mem_data( bio, &data );
    BIO_get_mem_data( bio, &data );
    return std::string( data, length );
  }


  size_t writeSSLErrors( std::ostream& out, char terminator ) {
    size_t count = 0;
    unsigned long error = 0;
    // https://www.openssl.org/docs/man1.0.2/man3/ERR_error_string.html buf must be at least 120 bytes
    char errbuf[200];
    while ( ( error = ERR_get_error() ) ) {
      ERR_error_string_n( error, errbuf, sizeof(errbuf) );
      out << errbuf;
      if ( terminator ) out << terminator;
      count++;
    }
    return count;
  }

  std::string getSSLErrors( char terminator ) {
    std::stringstream ss;
    if ( writeSSLErrors( ss, terminator ) ) {
      return ss.str();
    } else return "";
  }

  std::string fileReadString( const std::string &filename ) {
    std::ifstream ifs( filename.c_str() );
    std::string line = "";
    if ( ifs.good() ) {
      getline( ifs, line );
    } else throw_Exception( Puts() << "failed to open '" << filename << "'" );
    return line;
  }

  std::vector<std::string> fileReadStrings( const std::string &filename ) {
    std::ifstream ifs( filename.c_str() );
    std::vector<std::string> tmp;
    std::string line = "";
    getline( ifs, line );
    if ( ifs.good() ) throw_Exception( Puts() << "failed to open '" << filename << "'" );
    while ( ifs.good() ) {
      tmp.push_back( line );
      getline( ifs, line );
    }
    return tmp;
  }

  std::vector<std::string> fileReadStrings( const std::string &filename, const std::regex& exp ) {
    std::ifstream ifs( filename.c_str() );
    std::vector<std::string> tmp;
    std::string line = "";
    getline( ifs, line );
    if ( !ifs.good() ) throw_Exception( Puts() << "failed to open '" << filename << "'" );
    while ( ifs.good() ) {
      std::smatch m;
      if ( std::regex_match(line, m, exp ) ) tmp.push_back( line );
      getline( ifs, line );
    }
    return tmp;
  }

  bool fileReadAccess( const std::string& path ) {
    bool result = true;
    struct stat buf;
    int r = stat( path.c_str(), &buf );
    if ( r != 0 ) return false;
    result = S_ISREG(buf.st_mode);
    result = result && ( !access( path.c_str(), F_OK | R_OK ) );
    return result;
  }

}
