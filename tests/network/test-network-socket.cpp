#include <iostream>
#include <dodo.hpp>

using namespace dodo;

bool test1() {
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  network::Socket sock( true, sock_params );
  try {

    std::cout << "TTL     : " << sock.getTTL() << std::endl;
    std::cout << "sendbuf : " << sock.getSendBufSize() << std::endl;
    std::cout << "recvbuf : " << sock.getReceiveBufSize() << std::endl;

    sock.setTTL( 33 );
    sock.setSendBufSize( 16384 );
    sock.setReceiveBufSize( 16384 );
    std::cout << "TTL     : " << sock.getTTL() << std::endl;
    std::cout << "sendbuf : " << sock.getSendBufSize() << std::endl;
    std::cout << "recvbuf : " << sock.getReceiveBufSize() << std::endl;
  }
  catch ( common::Exception &e ) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return sock.getTTL() == 33 &&
         sock.getSendBufSize() == 16384*2 &&
         sock.getReceiveBufSize() == 16384*2 &&
         sock.getBlocking();
}

bool test2() {
  network::SocketParams sock_params = network::SocketParams( network::SocketParams::afINET6,
                                                             network::SocketParams::stSTREAM,
                                                             network::SocketParams::pnTCP );
  network::Socket sock( true, sock_params );
  try {

    std::cout << "TTL     : " << sock.getTTL() << std::endl;
    std::cout << "sendbuf : " << sock.getSendBufSize() << std::endl;
    std::cout << "recvbuf : " << sock.getReceiveBufSize() << std::endl;

    sock.setTTL( 33 );
    sock.setSendBufSize( 16384 );
    sock.setReceiveBufSize( 16384 );
    std::cout << "TTL     : " << sock.getTTL() << std::endl;
    std::cout << "sendbuf : " << sock.getSendBufSize() << std::endl;
    std::cout << "recvbuf : " << sock.getReceiveBufSize() << std::endl;
  }
  catch ( common::Exception &e ) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return sock.getTTL() == 33 &&
         sock.getSendBufSize() == 16384*2 &&
         sock.getReceiveBufSize() == 16384*2 &&
         sock.getBlocking();
}


int main() {
  std::cout << BuildEnv::getDescription();
  bool ok = true;

  ok = ok && test1();
  if ( !ok ) return 1;

  ok = ok && test2();
  if ( !ok ) return 1;

  return 0;
}