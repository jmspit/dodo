#include <iostream>
#include <dodo.hpp>
#include "service_tcpserver.hpp"

using namespace dodo;
using namespace dodo::common;
using namespace std;

class MyApp : public Application {
  public:
    MyApp( const StartParameters &param ) : Application( param ) {
    }

    virtual ~MyApp() {
      delete listener_;
    }

    virtual int run() {
      try {
        listener_ = new TCPListener( Config::getConfig()->getYAML()["server"] );
        listener_->start( new Server(*listener_) );
        while ( !hasStopRequest() ) {
          std::this_thread::sleep_for( std::chrono::milliseconds(50) );
        }
        listener_->stop();
        listener_->wait();
        log_Info( Config::getConfig()->getAppName() << " finished" );
      }
      catch ( const std::exception &e ) {
        log_Fatal( e.what() );
      }
      return 0;
    }
  protected:
    TCPListener* listener_;
};


void opimized_out( const std::string &s ) {
  //cout << s << endl;
}

int main( int argc, char* argv[], char** envp ) {
  try {
    MyApp app( { "conf/config.yaml", argc, argv, envp } );
    return app.run();
  }
  catch ( const std::exception &e ) {
    cerr << "main::catch std::exception : " << e.what() << endl;
    return 1;
  }
}