#ifndef dodo_common_exception_hpp
#define dodo_common_exception_hpp

#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>
#include <set>

#include <common/systemerror.hpp>

namespace dodo::common {

  /**
   * Any thing that can cast to a string that describes it to humans for debugging purposes.
   * Such things can be passed as is to Exception and display accordingly.
   */
  class DebugObject {
    public:
      DebugObject() {};

      virtual ~DebugObject() {};

      virtual std::string debugString() const;

      virtual std::string debugDetail() const { return ""; };

    protected:
      std::string debugHeader() const;

  };

  class Exception : public std::runtime_error {
    public:
      Exception( const std::string &file,
                 unsigned int line,
                 const std::string &what );
      Exception( const std::string &file,
                 unsigned int line,
                 const std::string &what,
                 const DebugObject* thing );
      virtual ~Exception();
      virtual const char* what() const noexcept;
      const std::string& getFile() const { return file_; };
      unsigned int getLine() const { return line_; };
    protected:
      std::string  file_;
      unsigned int line_;
      std::string  msg_;
  };

  class SystemException : public Exception {
    public:
      SystemException( const std::string &file,
                       unsigned int line,
                       const std::string &what,
                       const dodo::common::SystemError &error );
      SystemException( const std::string &file,
                       unsigned int line,
                       const std::string &what,
                       const dodo::common::SystemError &error,
                       const DebugObject* thing );
  };

  #define throw_Exception( what ) throw dodo::common::Exception( __FILE__, __LINE__, what )
  #define throw_ExceptionObject( what, thing ) throw dodo::common::Exception( __FILE__, __LINE__, what, thing )
  #define throw_SystemException( what, errno ) throw dodo::common::SystemException( __FILE__, __LINE__, what, errno )
  #define throw_SystemExceptionObject( what, errno, thing ) throw dodo::common::SystemException( __FILE__, __LINE__, what, errno, thing )

};

#endif