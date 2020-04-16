#ifndef dodo_hpp
#define dodo_hpp

#include <buildenv.hpp>
#include <common/exception.hpp>
#include <common/puts.hpp>
#include <common/systemerror.hpp>
#include <common/util.hpp>
#include <network/network.hpp>
#include <threads/mutex.hpp>
#include <threads/thread.hpp>

/**
 * A C++ platform library to eliminate red tape and provide a platform to build
 * any type of Linux service.
 */
namespace dodo {

  void initLibrary() {
    network::initLibrary();
  }

  void closeLibrary() {
    network::closeLibrary();
  }

}

#endif