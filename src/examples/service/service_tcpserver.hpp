#ifndef service_tcpserver_hpp
#define service_tcpserver_hpp

#include <dodo.hpp>

using namespace dodo::network;


class Server : public TCPServer {
  public:
    Server(TCPListener &listener) : TCPServer(listener) {}

    virtual TCPServer* addServer() { return new Server( listener_ ); }

    virtual bool handShake( BaseSocket *socket, ssize_t &received, ssize_t &sent ) {
      return true;
    }

    virtual bool readSocket( BaseSocket *socket, ssize_t &received, ssize_t &sent ) {
      std::string request;
      SystemError error = socket->receiveLine( request );
      received = request.length() + 1;
      sent = request.length() + 1;
      if ( error == SystemError::ecOK ) {
        log_Trace( "socket " << socket->getFD() << " received : '" << request << "'!" );
        error = socket->sendLine( request, false );
        return error == SystemError::ecOK;
      }
      return false;
    }

    virtual void shutDown( BaseSocket *socket ) {
    }
};

#endif

