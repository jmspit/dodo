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
   * Interface to objects that support dumping their state to a string.
   */
  class DebugObject {
    public:

      /**
       * Default constructor does nothing.
       */
      DebugObject() {};

      /**
       * Destructor does nothing.
       */
      virtual ~DebugObject() {};

      /**
       * Return the object dump to string. debugHeader() and debugDetail() is integrated in the dump.
       * @see debugHeader()
       * @see debugDetail()
       */
      std::string debugString() const;

    protected:

      /**
       * Descendant classes can override to dump details speciufic to the class. By default, returns nothing.
       */
      virtual std::string debugDetail() const { return ""; };

      /**
       * Generates a debug header (address of this object and a demangled class name.
       */
      std::string debugHeader() const;

  };

  /**
   * Exception with source code origin / a (file, line) tuple.
   */
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

  /**
   * Decsending from Exception, speciically for execptions based on a system error code.
   * @see SystemError
   */
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