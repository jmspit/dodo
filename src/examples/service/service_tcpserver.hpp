#ifndef service_tcpserver_hpp
#define service_tcpserver_hpp

#include <dodo.hpp>

using namespace dodo::network;

class Server : public TCPServer {
  public:
    Server(TCPListener &listener) : TCPServer(listener) {}

    virtual TCPServer* addServer() { return new Server( listener_ ); }

    virtual bool handShake( BaseSocket *socket ) {
      return true;
    }

    virtual bool requestResponse( BaseSocket *socket ) {
      std::string request;
      SystemError error = socket->receiveString( request );
      if ( error == SystemError::ecOK ) {
        error = socket->sendString( request, false );
        return error == SystemError::ecOK;
      }
      return false;
    }

    virtual void shutDown( BaseSocket *socket ) {
    }
};

#endif

