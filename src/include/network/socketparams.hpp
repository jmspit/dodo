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
 * @file socketparams.hpp
 * Defines the dodo::network::SocketParams class.
 */

#ifndef network_socketparams_hpp
#define network_socketparams_hpp

#include <netdb.h>
#include <string>
#include <sstream>

namespace dodo::network {

  using namespace std;

  /**
   * Socket parameters - the family (domain), socket type and protocol triplet.
   */
  class SocketParams {
    public:

      enum AddressFamily {
        afLOCAL     = AF_LOCAL,
        afINET      = AF_INET,
        afINET6     = AF_INET6,
        afIPX       = AF_IPX,
        afNETLINK   = AF_NETLINK,
        afX25       = AF_X25,
        afAX25      = AF_AX25,
        afATMPVC    = AF_ATMPVC,
        afAPPLETALK = AF_APPLETALK,
        afPACKET    = AF_PACKET,
        afALG       = AF_ALG,
        afUNSPEC    = AF_UNSPEC
      };

      enum SocketType {
        stSTREAM    = SOCK_STREAM,
        stDGRAM     = SOCK_DGRAM,
        stSEQPACKET = SOCK_SEQPACKET,
        stRAW       = SOCK_RAW,
        stRDM       = SOCK_RDM,
        stPACKET    = SOCK_PACKET
      };


      /**
       * IANA Protocol number.
       * https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
       */
      enum ProtocolNumber {
        pnHOPOPT      = 0,
        pnICMP        = IPPROTO_ICMP,
        pnICMPV6      = IPPROTO_ICMPV6,
        pnIGMP        = 2,
        pnGGP         = 3,
        pnIPv4        = 4,
        pnST          = 5,
        pnTCP         = 6,
        pnCBT         = 7,
        pnEGP         = 8,
        pnIGP         = 9,
        pnBBN_RCC_MON = 10,
        pnNVP_II      = 11,
        pnPUP         = 12,
        pnARGUS       = 13,
        pnEMCON       = 14,
        pnXNET        = 15,
        pnCHAOS       = 16,
        pnUDP         = 17,
        pnMUX         = 18,
        pnDCN_MEAS    = 19,
        pnHMP         = 20,
        pnPRM         = 21,
        pnXNS_IDP     = 22,
        pnTRUNK_1     = 23,
        pnTRUNK_2     = 24,
        pnLEAF_1      = 25,
        pnLEAF_2      = 26,
        pnRDP         = 27,
        pnIRTP        = 28,
        pnISO_TP4     = 29,
        pnNETBLT      = 30,
        pnMFE_NSP     = 31,
        pnMERIT_INP   = 32,
        pnDCCP        = 33,
        pn3PC         = 34,
        pnIDPR        = 35,
        pnXTP         = 36,
        pnDDP         = 37,
        pnIDPR_CMTP   = 38
      };

      /**
       * Default SocketParams to AF_INET6, SOCK_STREAM, protocol 0
       */
      SocketParams() : family_(afINET6), sockettype_(stSTREAM), protocol_(pnHOPOPT) {};

      SocketParams( AddressFamily family ) : family_(family), sockettype_(stSTREAM),protocol_(pnHOPOPT) {};

      SocketParams( AddressFamily family, SocketType sockettype ) : family_(family), sockettype_(sockettype),protocol_(pnHOPOPT) {};

      SocketParams( AddressFamily family, SocketType sockettype, ProtocolNumber protocol ) : family_(family), sockettype_(sockettype),protocol_(protocol) {};

      AddressFamily getAddressFamily() const { return family_; };

      SocketType getSocketType() const { return sockettype_; };

      ProtocolNumber getProtocol() const { return protocol_; };

      void setAddressFamily( AddressFamily family ) { family_ = family; };

      void setSocketType( SocketType sockettype ) { sockettype_ = sockettype; };

      void setProtocol( ProtocolNumber protocol ) { protocol_ = protocol; };

      string asString() const {
        stringstream ss;
        ss << familyString( family_ ) << " ";
        ss << socketTypeString( sockettype_ ) << " ";
        ss << protocolString( protocol_ );
        return ss.str();
      };

      static string familyString( AddressFamily family ) {
        stringstream ss;
        switch ( family ) {
          case AF_LOCAL     : ss << "AF_LOCAL"; break;
          case AF_INET      : ss << "AF_INET"; break;
          case AF_INET6     : ss << "AF_INET6"; break;
          case AF_IPX       : ss << "AF_IPX"; break;
          case AF_NETLINK   : ss << "AF_NETLINK"; break;
          case AF_X25       : ss << "AF_X25"; break;
          case AF_AX25      : ss << "AF_AX25"; break;
          case AF_ATMPVC    : ss << "AF_ATMPVC"; break;
          case AF_APPLETALK : ss << "AF_APPLETALK"; break;
          case AF_PACKET    : ss << "AF_PACKET"; break;
          case AF_ALG       : ss << "AF_ALG"; break;
          case AF_UNSPEC    : ss << "AF_UNSPEC"; break;
          default           : ss << "unhandled protocol family"; break;
        }
        return ss.str();
      };

      static string socketTypeString( SocketType sockettype ) {
        stringstream ss;
        switch ( sockettype ) {
          case SOCK_STREAM    : ss << "SOCK_STREAM"; break;
          case SOCK_DGRAM     : ss << "SOCK_DGRAM"; break;
          case SOCK_SEQPACKET : ss << "SOCK_SEQPACKET"; break;
          case SOCK_RAW       : ss << "SOCK_RAW"; break;
          case SOCK_RDM       : ss << "SOCK_RDM"; break;
          case SOCK_PACKET    : ss << "SOCK_PACKET"; break;
          default             : ss << "unhandled socket type"; break;
        }
        return ss.str();
      };

      static string protocolString( ProtocolNumber protocol ) {
        stringstream ss;
        switch ( protocol ) {
          case pnHOPOPT      : ss << "HOPOPT"; break;
          case pnICMP        : ss << "ICMP"; break;
          case pnIGMP        : ss << "IGMP"; break;
          case pnGGP         : ss << "GGP"; break;
          case pnIPv4        : ss << "IPv4"; break;
          case pnST          : ss << "ST"; break;
          case pnTCP         : ss << "TCP"; break;
          case pnCBT         : ss << "CBT"; break;
          case pnEGP         : ss << "EGP"; break;
          case pnIGP         : ss << "IGP"; break;
          case pnBBN_RCC_MON : ss << "BBN-RCC-MON"; break;
          case pnNVP_II      : ss << "NVP-II"; break;
          case pnPUP         : ss << "PUP"; break;
          case pnARGUS       : ss << "ARGUS"; break;
          case pnEMCON       : ss << "EMCON"; break;
          case pnXNET        : ss << "XNET"; break;
          case pnCHAOS       : ss << "CHAOS"; break;
          case pnUDP         : ss << "UDP"; break;
          case pnMUX         : ss << "MUX"; break;
          case pnDCN_MEAS    : ss << "DCN_MEAS"; break;
          case pnHMP         : ss << "HMP"; break;
          default          : ss << "unknown"; break;
        }
        return ss.str();
      };

    private:
      AddressFamily family_;
      SocketType sockettype_;
      ProtocolNumber protocol_;
  };

};

#endif
