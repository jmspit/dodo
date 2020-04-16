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
   * An Exception is thrown in exceptional circumstances, and its occurence should generally imply that the program
   * should stop, as it has entered a state it was never designed to handle.
   *
   * When used in conjunction with the throw_Exception() and throw_SystemException() macros, the source file
   * and line number where the Exception is thrown are picked up automatically.
   */
  class Exception : public std::runtime_error {
    public:
      /**
       * Construct an Exception. Use the throw_Exception() macro to construct and throw Exceptions.
       */
      Exception( const std::string &file,
                 unsigned int line,
                 const std::string &what );
      /**
       * Construct an Exception. Use the throw_ExceptionObject() macro to construct and throw Exceptions.
       */
      Exception( const std::string &file,
                 unsigned int line,
                 const std::string &what,
                 const DebugObject* thing );

      virtual ~Exception();

      /**
       * Return the exception message.
       * @return the exception message.
       */
      virtual const char* what() const noexcept;

      /**
       * Return the source file where the exception was thrown.
       * @return The source file where the exception was thrown.
       */
      const std::string& getFile() const { return file_; };

      /**
       * Return the line number in the source file where the exception was thrown.
       * @return The line numbver in the source file where the exception was thrown.
       */
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
    protected:
      dodo::common::SystemError error_;
  };

  /**
   * throws an Exception, passes __FILE__ and __LINE__ to constructor.
   * @param what The exception message as std::string.
   */
  #define throw_Exception( what ) throw dodo::common::Exception( __FILE__, __LINE__, what )
  #define throw_ExceptionObject( what, thing ) throw dodo::common::Exception( __FILE__, __LINE__, what, thing )
  #define throw_SystemException( what, errno ) throw dodo::common::SystemException( __FILE__, __LINE__, what, errno )
  #define throw_SystemExceptionObject( what, errno, thing ) throw dodo::common::SystemException( __FILE__, __LINE__, what, errno, thing )

};

#endif