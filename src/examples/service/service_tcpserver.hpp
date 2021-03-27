#ifndef service_tcpserver_hpp
#define service_tcpserver_hpp

#include <dodo.hpp>

using namespace dodo::network;
using namespace dodo::common;


class Server : public TCPServer {
  public:
    explicit Server(TCPListener &listener) : TCPServer(listener) {}

    virtual TCPServer* addServer() { return new Server( listener_ ); }

    virtual bool handShake( BaseSocket *socket, ssize_t &received, ssize_t &sent ) {
      return true;
    }

    virtual SystemError readSocket( TCPListener::SocketWork &work, ssize_t &sent ) {
      SystemError error;
      const Bytes &buf = work.data->getReadBuffer();
      if ( buf.getSize() > 0 ) {
        if ( buf.getOctet(buf.getSize()-1) == '\n' ) {
          error = work.socket->send( buf.getArray(), buf.getSize() );
          work.data->clearBuffer();
          return error;
        } else if ( buf.getOctet(0) == 0xff || buf.getOctet(0) == 0x04 ) {
          return SystemError::ecECONNABORTED;
        } else return SystemError::ecEAGAIN;
      } else return SystemError::ecEAGAIN;
    }

    virtual void shutDown( BaseSocket *socket ) {
    }
};

#endif
