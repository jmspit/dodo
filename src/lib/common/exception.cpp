#include <common/exception.hpp>
#include <iostream>

#if CMAKE_CXX_COMPILER_ID == GNU
  #include <cxxabi.h>
#endif

namespace dodo::common {

  std::string DebugObject::debugString() const {
    std::stringstream ss;
    ss << debugHeader();
    ss << debugDetail();
    return ss.str();
  }

  std::string DebugObject::debugHeader() const {
    std::stringstream ss;

    #if CMAKE_CXX_COMPILER_ID == GNU
    int status;
    char * demangled = abi::__cxa_demangle(typeid(*this).name(),0,0,&status);
    ss << demangled << " ";
    free(demangled);
    #else
    ss << typeid(*this).name() << " ";
    #endif

    ss << "address " << std::hex << (void*)this << std::endl;
    return ss.str();
  }

  Exception::Exception( const std::string &file, unsigned int line, const std::string &what ) :
    std::runtime_error(what), file_(file), line_(line) {
    std::stringstream ss;
    ss << file_ << ":" << line_ << " " << what;
    msg_ = ss.str();
  }

  Exception::Exception( const std::string &file, unsigned int line, const std::string &what, const DebugObject* thing ) :
    std::runtime_error(what), file_(file), line_(line) {
    std::stringstream ss;
    ss << file_ << ":" << line_ << " " << what;
    ss << std::endl << " " << thing->debugString();
    msg_ = ss.str();
  }

  Exception::~Exception() {
  }

  const char* Exception::what() const noexcept {
    return msg_.c_str();
  }

  SystemException::SystemException( const std::string &file,
                                    unsigned int line,
                                    const std::string &what,
                                    const SystemError &error )
    : Exception( file, line, what + " : " + error.asString() ) {
    error_ = error;
  }

  SystemException::SystemException( const std::string &file,
                                    unsigned int line,
                                    const std::string &what,
                                    const SystemError &error,
                                    const DebugObject* thing )
    : Exception( file,
                 line,
                 common::Puts() << what << " : " << error.asString() << common::Puts::endl() << thing->debugString() ) {
    error_ = error;
  }

}