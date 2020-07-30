#ifndef service_tcpserver_hpp
#define service_tcpserver_hpp

#include <dodo.hpp>

using namespace dodo::network;
using namespace dodo::common;


class Server : public TCPServer {
  public:
    Server(TCPListener &listener) : TCPServer(listener) {}

    virtual TCPServer* addServer() { return new Server( listener_ ); }

    virtual bool handShake( BaseSocket *socket, ssize_t &received, ssize_t &sent ) {
      return true;
    }

    virtual SystemError readSocket( TCPListener::SocketWork &work, ssize_t &sent ) {
      SystemError error;
      const OctetArray &buf = work.data->getReadBuffer();
      if ( buf.size > 0 ) {
        if ( buf.array[buf.size-1] == '\n' ) {
          error = work.socket->send( buf.array, buf.size );
          work.data->clearBuffer();
          return error;
        } else if ( buf.array[0] == 0xff || buf.array[0] == 0x04 ) {
          return SystemError::ecECONNABORTED;
        } else return SystemError::ecEAGAIN;
      } else return SystemError::ecEAGAIN;
    }

    virtual void shutDown( BaseSocket *socket ) {
    }
};

#endif

