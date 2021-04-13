#include <dodo.hpp>


using namespace dodo;
using namespace std;

class TestCache : public common::Cache<int, std::string> {
  public:
    TestCache( size_t max_size, std::chrono::seconds life_time ) : Cache<int, std::string>(max_size,life_time) {}
  protected:
    virtual bool load( const int &key, std::string &value ) {
      std::stringstream ss;
      ss << "retrieving value for key " << key;
      value = ss.str();
      return true;
    }
};

int main() {
  TestCache cache( 38, 40s );
  for ( int i = 0; i < 200; i++ ) {
    std::string value;
    cache.get( rand() % 40, value );
    //cout << value << endl;
    std::this_thread::sleep_for (std::chrono::milliseconds(rand() % 1000));
    std::cout << "." << std::flush;
  }
  std::cout << std::endl;
  cout << "hit     " << cache.getHits() << endl;
  cout << "miss    " << cache.getMisses() << endl;
  cout << "expired " << cache.getExpiries() << endl;
  return 0;
}