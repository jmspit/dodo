#include <iostream>
#include <dodo.hpp>

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cmath>
#include <signal.h>
#include <climits>


using namespace dodo;
using namespace std;

#define PING_PKT_S 60000

#define LATENCY_PRECISION 2

#define RECEIVE_TIMEOUT 2.0

#define PING_COUNT 20

#define PING_SLEEP_INTERVAL 1000000

struct ping_pkt {
  struct icmphdr hdr;
  char msg[PING_PKT_S-sizeof(struct icmp6_hdr)-sizeof(struct iphdr)];
};

struct reply_pkt {
  iphdr    ip_hdr;
  ping_pkt icmp_reply;
};

struct ping_pkt6 {
    struct icmp6_hdr hdr;
    char msg[PING_PKT_S-sizeof(struct icmp6_hdr)];
};

struct reply_pkt6 {
  //ip6_hdr  ip_hdr;
  ping_pkt6 icmp_reply;
};

struct ping_stats {
  size_t total_request;
  size_t total_reply;
  double min_;
  double max_;
  double mu;
  double sq;

  ping_stats() : total_request(0),
                 total_reply(0),
                 min_(0.0),
                 max_(0.0),
                 mu(0.0),
                 sq(0.0) {};

  double min() const { return min_; };
  double max() const { return max_; };
  double mean() const { return mu; };
  double stddev() const { return total_reply > 1 && sq != 0.0 ? sqrt( sq/(double)total_reply ) : 0.0; };
  void success( double v ) {
    if ( v < min_ || !total_request ) min_ = v;
    if ( v > max_ || !total_request ) max_ = v;
    total_request++;
    total_reply++;
    double mu_new = mu + (v - mu)/(double)total_reply;
    sq += (v - mu) * (v - mu_new);
    mu = mu_new;
  }
  void fail() {
    total_request++;
  }
};

struct runtime_options {
  /**
   * number of microseconds between pings.
   */
  unsigned int ping_interval;

  /**
   * display precision for decimal numbers
   */
  unsigned int precision;

  /**
   * Number of pings to do per address.
   */
  unsigned int ping_count;

  /**
   * ICMP packet size.
   */
  unsigned int ping_packet_size;

  /**
   * Time to live.
   */
  unsigned int ttl;

  /**
   * Second before giving up on a ICMP echo reply.
   */
  double receive_timeout;

  bool do_ipv4;
  bool do_ipv6;

  runtime_options() : ping_interval(1000000),
                      precision(3),
                      ping_count(UINT_MAX),
                      ping_packet_size(64),
                      ttl(56),
                      receive_timeout( 10.0 ),
                      do_ipv4(true),
                      do_ipv6(true) {};
  void dump() {
    cout << "ping_interval    : " << ping_interval << endl;
    cout << "precision        : " << precision << endl;
    cout << "ping_count       : " << ping_count << endl;
    cout << "ping_packet_size : " << ping_packet_size << endl;
    cout << "ttl              : " << ttl << endl;
    cout << "receive_timeout  : " << receive_timeout << endl;
  }
};

string hostname = "localhost";
network::AddrInfo addrinfo;
runtime_options options;
bool request_stop = false;


/**
 * http://www.faqs.org/rfcs/rfc1071.html
 */
uint16_t checksum( void *b, int len ) {
  uint16_t *buf = (uint16_t *)b;
  unsigned int sum = 0 ;
  uint16_t result;

  for ( sum = 0; len > 1; len -= 2 ) sum += *buf++;
  if ( len == 1 ) sum += *(unsigned char*)buf;
  sum = ( sum >> 16 ) + ( sum & 0xFFFF );
  sum += ( sum >> 16 );
  result = (uint16_t) ~sum;
  return result;
}

void initPacket( ping_pkt *pkt ) {
  bzero( pkt, sizeof(struct ping_pkt) );
  pkt->hdr.type = ICMP_ECHO;
  pkt->hdr.un.echo.id = htons( (uint16_t)getpid() );
  strncpy( pkt->msg, "freedom", 8 );
  pkt->msg[sizeof(pkt->msg)-1] = 0;
}

void nextPacket( ping_pkt *pkt, uint16_t sq ) {
  pkt->hdr.un.echo.sequence = htons( sq );
  pkt->hdr.checksum = 0;
  pkt->hdr.checksum = checksum( pkt, options.ping_packet_size );
}

void initPacket( ping_pkt6 *pkt ) {
  bzero( pkt, sizeof(struct ping_pkt6) );
  pkt->hdr.icmp6_type = ICMP6_ECHO_REQUEST;
  pkt->hdr.icmp6_id = htons( (uint16_t)getpid() );
  strncpy( pkt->msg, "freedom", 8 );
  pkt->msg[sizeof(pkt->msg)-1] = 0;
}

void nextPacket( ping_pkt6 *pkt, uint16_t sq ) {
  pkt->hdr.icmp6_seq = htons( sq );
  pkt->hdr.icmp6_cksum = 0;
  //pkt->hdr.icmp6_cksum = checksum( pkt, sizeof(struct ping_pkt6) );
  pkt->hdr.icmp6_cksum = checksum( pkt, options.ping_packet_size );
}

void dumpPacket( ping_pkt *pkt ) {
  cout << "INET" << endl;
  cout << "type     : " << (int)pkt->hdr.type << endl;
  cout << "code     : " << ntohs( pkt->hdr.code ) << endl;
  cout << "checksum : " << ntohs( pkt->hdr.checksum ) << endl;
  cout << "id       : " << ntohs( pkt->hdr.un.echo.id ) << endl;
  cout << "sequence : " << ntohs( pkt->hdr.un.echo.sequence ) << endl;
  cout << "msg      : " << pkt->msg << endl;
}

void dumpPacket( ping_pkt6 *pkt ) {
  cout << "INET6" << endl;
  cout << "type     : " << (int)pkt->hdr.icmp6_type << endl;
  cout << "code     : " << ntohs( pkt->hdr.icmp6_code ) << endl;
  cout << "checksum : " << ntohs( pkt->hdr.icmp6_cksum ) << endl;
  cout << "id       : " << ntohs( pkt->hdr.icmp6_id ) << endl;
  cout << "sequence : " << ntohs( pkt->hdr.icmp6_seq ) << endl;
  cout << "msg      : " << pkt->msg << endl;
}

void writeStatsLine( const network::Address &addr,
                     const ping_stats &stats,
                     const string &reverse ) {
  cout << "--- " << reverse << " " << addr.asString() << " ping statistics ---" << endl
       << stats.total_request << " packets transmitted, " << stats.total_reply << " received, "
       << fixed << setprecision(0)
       << (1.0-(double)stats.total_reply/(double)stats.total_request)*100.0 << "% packet loss" << endl
       << "rtt min/avg/max/sdev = "
       << fixed << setprecision(options.precision)
       << stats.min() * 1000.0  << "/"
       << stats.mean() * 1000.0  << "/"
       << stats.max() * 1000.0  << "/"
       << stats.stddev() * 1000.0 << " ms "
       << endl << endl;
}

void ping( const network::AddrInfoItem &item, ping_stats &stats ) {
  struct ping_pkt pckt;
  uint16_t msg_count = 0;
  reply_pkt reply;
  ssize_t bytes_received = 0;
  common::StopWatch sw;
  common::SystemError error;

  string reverse = item.address.asString();
  error = item.address.getNameInfo( reverse );
  if ( error != common::SystemError::ecOK && error != common::SystemError::ecEAI_NONAME )
    throw_SystemException( "address.getNameInfo", error );

  network::SocketParams sock_params = network::SocketParams( item.params.getAddressFamily(),
                                                             network::SocketParams::stRAW,
                                                             network::SocketParams::pnICMP );
  network::Socket socket( true, sock_params );
  socket.setTTL( options.ttl );
  socket.setReceiveTimeout( options.receive_timeout );
  socket.setSendBufSize( PING_PKT_S + 100 );
  socket.setReceiveBufSize( PING_PKT_S + 100 );

  initPacket( &pckt );
  double elapsed = 0.0;
  while ( msg_count < options.ping_count && !request_stop ) {
    nextPacket( &pckt, msg_count++ );

    cout << "icmp " << item.address.asString()
         << " id " << ntohs( pckt.hdr.un.echo.id )
         << " seq " << ntohs( pckt.hdr.un.echo.sequence );
    error = socket.sendTo( item.address, &pckt, options.ping_packet_size );
    sw.start();
    if ( error != common::SystemError::ecOK ) throw_SystemException( "socket.sendTo", error );
    do {
      error = socket.receive( &reply, options.ping_packet_size+sizeof(reply.ip_hdr), bytes_received );
      elapsed = sw.getElapsedSeconds();
    } while ( error == common::SystemError::ecOK &&
              ( pckt.hdr.un.echo.id != reply.icmp_reply.hdr.un.echo.id ||
                pckt.hdr.un.echo.sequence != reply.icmp_reply.hdr.un.echo.sequence ) );
    sw.stop();
    if ( error == common::SystemError::ecOK && reply.icmp_reply.hdr.code == 0 ) {
      stats.success( elapsed );
      cout << " " << bytes_received - sizeof(iphdr) << " bytes "
           << fixed << setprecision(options.precision) << elapsed * 1000.0 << " ms" << endl;
    } else if ( error == common::SystemError::ecOK ) {
      stats.fail();
      cout << " code " << ntohs( reply.icmp_reply.hdr.code ) << endl;
    } else {
      stats.fail();
      cout << " " <<  error.asString() << endl;
    }

    if ( msg_count < options.ping_count ) usleep(options.ping_interval);
  }
  writeStatsLine( item.address, stats, reverse );
}

void ping6( const network::AddrInfoItem &item, ping_stats &stats ) {
  struct ping_pkt6 pckt;
  uint16_t msg_count = 0;
  reply_pkt6 reply;
  ssize_t bytes_received = 0;
  common::StopWatch sw;
  common::SystemError error;

  string reverse = item.address.asString();
  error = item.address.getNameInfo( reverse );
  if ( error != common::SystemError::ecOK && error != common::SystemError::ecEAI_NONAME )
    throw_SystemException( "address.getNameInfo", error );

  network::SocketParams sock_params = network::SocketParams( item.params.getAddressFamily(),
                                                             network::SocketParams::stRAW,
                                                             network::SocketParams::pnICMPV6 );
  network::Socket socket( true, sock_params );
  socket.setTTL( options.ttl );
  socket.setReceiveTimeout( options.receive_timeout );
  socket.setSendBufSize( PING_PKT_S + 100 );
  socket.setReceiveBufSize( PING_PKT_S + 100 );

  initPacket( &pckt );
  while ( msg_count < options.ping_count && !request_stop ) {
    nextPacket( &pckt, msg_count++ );

    cout << "icmp6 " << item.address.asString()
         << " id " << ntohs( pckt.hdr.icmp6_id )
         << " seq " << ntohs( pckt.hdr.icmp6_seq );
    error = socket.sendTo( item.address, &pckt, options.ping_packet_size );
    sw.start();
    if ( error != common::SystemError::ecOK ) throw_SystemException( "socket.sendTo", error );

    do {
      error = socket.receive( &reply, options.ping_packet_size, bytes_received );
    } while ( error == common::SystemError::ecOK &&
              ( reply.icmp_reply.hdr.icmp6_type != ICMP6_ECHO_REPLY ||
                reply.icmp_reply.hdr.icmp6_id !=  pckt.hdr.icmp6_id ||
                reply.icmp_reply.hdr.icmp6_seq != pckt.hdr.icmp6_seq ) );
    sw.stop();
    if ( error == common::SystemError::ecOK &&
         ( reply.icmp_reply.hdr.icmp6_type == ICMP6_ECHO_REPLY ) ) {
      stats.success( sw.getElapsedSeconds() );
      cout << " " << bytes_received << " bytes "
           << fixed << setprecision(options.precision) << sw.getElapsedSeconds() * 1000.0 << " ms" << endl;
    } else if ( error == common::SystemError::ecOK ) {
      stats.fail();
      cout << " icmp6 error " <<  (int)reply.icmp_reply.hdr.icmp6_type << endl;
    } else {
      stats.fail();
      cout << " " <<  error.asString() << endl;
    }
    if ( msg_count < options.ping_count ) usleep(options.ping_interval);
  }
  writeStatsLine( item.address, stats, reverse );
}

void printHelp() {
  cout << "ping [options] destination" << endl << endl;
  cout << "  -c count      : number of ICMP requests to send, default 10" << endl;
  cout << "  -i interval   : seconds between ping (decimals allowed), default 1.0" << endl;
  cout << "  -p precision  : latency display precision, default 2" << endl;
  cout << "  -W timeout    : receive timeout in seconds, default 10" << endl;
  cout << "  -t ttl        : time to live, default 59" << endl;
  cout << "  -s packetsize : ICMP packet size, between 48 and 60000 bytes, default 64" << endl;
  cout << "  -4            : ping ipv4 adresses" << endl;
  cout << "  -6            : ping ipv6 adresses" << endl;
  cout << "  -h            : display help" << endl;
  cout << endl;
  cout << "Sends ICMP echo requests to destination, either an ipv4 or ipv6 address, or a DNS name." << endl;
  cout << "If a DNS name is given, all PTR addresses are pinged, unless limited by -4 or -6." << endl;
  cout << "Specifying both -4 and -6 has the same effect as specifying neither." << endl;
}


bool parseArgs( int argc, char* argv[] ) {
  int opt;
  while ( ( opt = getopt( argc, argv, "c:p:W:t:s:i:46h" ) ) != -1 ) {
    switch ( opt ) {
    case 'c':
      options.ping_count = atoi(optarg);
      break;
    case 'p':
      options.precision = atoi(optarg);
      break;
    case 'W':
      options.receive_timeout = strtod(optarg,0);
      break;
    case 't':
      options.ttl = atoi(optarg);
      break;
    case 's':
      options.ping_packet_size = atoi(optarg);
      if ( options.ping_packet_size < sizeof(ip6_hdr) + 8 ) options.ping_packet_size = sizeof(ip6_hdr) + 8;
      if ( options.ping_packet_size > 60000 ) options.ping_packet_size = 60000;
      break;
    case 'i':
      options.ping_interval = (unsigned int)(strtod(optarg,0)*1.0E6);
      break;
    case '4':
      options.do_ipv6 = false;
      break;
    case '6':
      options.do_ipv4 = false;
      break;
    case 'h':
      printHelp();
      exit(EXIT_SUCCESS);
      break;
    default: /* '?' */
      printHelp();
      exit(EXIT_FAILURE);
    }
  }
  if ( !options.do_ipv4 && !options.do_ipv6 ) { options.do_ipv4 = true; options.do_ipv6 = true; };
  if (optind >= argc) hostname = "localhost"; else hostname = argv[optind];
  return true;
}

void intHandler( int signal ) {
  request_stop = true;
}

int main( int argc, char* argv[] ) {
  if ( parseArgs( argc, argv ) ) {
    signal( SIGINT, intHandler );
    //options.dump();
    try {
      common::SystemError error = network::Address::getHostAddrInfo( hostname, addrinfo );
      if ( error != common::SystemError::ecOK ) throw_SystemException( "network::Address::getHostAddrInfo", error );
      size_t address_pinged = 0;
      for ( auto i : addrinfo.items ) {
        if ( ( i.address.getAddressFamily() == network::SocketParams::afINET ||
               i.address.getAddressFamily() == network::SocketParams::afINET6  ) &&
               i.params.getSocketType() == network::SocketParams::stRAW && !request_stop ) {
          ping_stats stats;
          if ( i.address.getAddressFamily() == network::SocketParams::afINET && options.do_ipv4 ) {
            ping( i, stats );
            address_pinged++;
          }
          if ( i.address.getAddressFamily() == network::SocketParams::afINET6 && options.do_ipv6 ) {
            ping6( i, stats );
            address_pinged++;
          }
        }
      }
      if ( !addrinfo.items.size() ) {
        cerr << "cannot resolve " << hostname << endl;
        return 1;
      }
      if ( !address_pinged ) {
        cerr << "no suitable address found for " << hostname << endl;
        return 1;
      }
      return 0;
    }
    catch ( std::runtime_error &e ) {
      cerr << e.what() << endl;
      return 1;
    }
  } else printHelp();
}