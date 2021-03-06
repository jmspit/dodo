#include <iostream>
#include <dodo.hpp>

using namespace dodo;
using namespace std;


class MyApp : public common::Application {
  public:
    explicit MyApp( const StartParameters &param ) : common::Application( param ) {}
    virtual int run() {
      while ( !hasStopRequest() ) {
        cout << "Hello world!" << endl;
        std::this_thread::sleep_for(2s);
      }
      return 0;
    }
};

int main( int argc, char* argv[], char** envp ) {
  try {
    MyApp app( { "sysinfo.yaml", argc, argv, envp } );
    return app.run();
  }
  catch ( const std::exception &e ) {
    cerr << e.what() << endl;
    return 1;
  }
}