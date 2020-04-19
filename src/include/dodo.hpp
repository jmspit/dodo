#ifndef dodo_hpp
#define dodo_hpp

#include <buildenv.hpp>
#include <common/common.hpp>
#include <network/network.hpp>
#include <threads/threads.hpp>

/**
 * A C++ platform interface to linux (network) services tailored for containarized deployment.
 */
namespace dodo {

  /**
   * Initialize the dodo library.
   */
  void initLibrary() {
    common::initLibrary();
    threads::initLibrary();
    network::initLibrary();
  }

  /**
   * Close the dod library.
   */
  void closeLibrary() {
    network::closeLibrary();
    threads::closeLibrary();
    common::closeLibrary();
  }

}

#endif