#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace dodo::common;
using namespace dodo::threads;
using namespace std;

class MyThread : public Thread {
  public:
    void stop() { stop_request_ = true; }
  protected:
    virtual void run() {
      size_t c = 0;
      while ( !stop_request_ ) {
        if ( c > 19 ) {
          Logger::getLogger()->log( Logger::LogLevel::Info, "ping!" );
          c = 0;
        } else c++;
        std::this_thread::sleep_for(200ms);
      }
    }

    bool stop_request_ = false;
};

class MyApp : public Application {
  public:
    MyApp( const StartParameters &param ) : Application( param ) {}
    virtual int run() {

      MyThread thread1;
      thread1.start();
      MyThread thread2;
      thread2.start();
      MyThread thread3;
      thread3.start();

      while ( !hasStopRequest() ) {
        std::this_thread::sleep_for(200ms);
      }
      log( Logger::LogLevel::Info, "stopping threads ..." );
      thread1.stop();
      thread2.stop();
      thread3.stop();
      thread1.wait();
      thread2.wait();
      thread3.wait();
      log( Logger::LogLevel::Info, "stopped, bye" );
      return 0;
    }
};


int main( int argc, char* argv[], char** envp ) {
  try {
    MyApp app( { "conf/config.yaml", argc, argv, envp } );
    return app.run();
  }
  catch ( const std::exception &e ) {
    cerr << "catch std::exception : " << e.what() << endl;
    return 1;
  }
}