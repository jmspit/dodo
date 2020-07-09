/*
 * This file is part of the dodo library (https://github.com/jmspit/dodo).
 * Copyright (c) 2019 Jan-Marten Spit.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file exception.hpp
 * Defines dodo::common::Exception.
 */

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
       * @return that string.
       * @see debugHeader()
       * @see debugDetail()
       */
      std::string debugString() const;

    protected:

      /**
       * Descendent classes can override to dump details speciufic to the class. By default, returns nothing.
       * @return The stirng.
       */
      virtual std::string debugDetail() const { return ""; };

      /**
       * Generates a debug header (address of this object and a demangled class name.
       * @return The string.
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
       * @param file The source file where the exception was raised.
       * @param line The line number where the exception was raised.
       * @param what The execption message.
       */
      Exception( const std::string &file,
                 unsigned int line,
                 const std::string &what );
      /**
       * Construct an Exception. Use the throw_ExceptionObject() macro to construct and throw Exceptions.
       * @param file The source file where the exception was raised.
       * @param line The line number where the exception was raised.
       * @param what The execption message.
       * @param thing The execption context.
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
      /** The source file */
      std::string  file_;
      /** The source line number */
      unsigned int line_;
      /** The exception message */
      std::string  msg_;
  };

  /**
   * Descending from Exception, exceptions based on a dodo::common::SystemError code.
   * @see SystemError
   */
  class SystemException : public Exception {
    public:

      /**
       * Constructor
       * @param file The source file where the exception was raised.
       * @param line The line number where the exception was raised.
       * @param what The execption message.
       * @param error The underlying SystemError.
       */
      SystemException( const std::string &file,
                       unsigned int line,
                       const std::string &what,
                       const dodo::common::SystemError &error );
      /**
       * Constructor
       * @param file The source file where the exception was raised.
       * @param line The line number where the exception was raised.
       * @param what The execption message.
       * @param error The underlying SystemError.
       * @param thing The DbugObject context to the error.
       */
      SystemException( const std::string &file,
                       unsigned int line,
                       const std::string &what,
                       const dodo::common::SystemError &error,
                       const DebugObject* thing );
    protected:
      /** The exception system error */
      dodo::common::SystemError error_;
  };

  /**
   * Throws an Exception, passes __FILE__ and __LINE__ to constructor.
   * @param what The exception message as std::string.
   */
  #define throw_Exception( what ) throw dodo::common::Exception( __FILE__, __LINE__, what )

  /**
   * Throws an Exception with DebugContext, passes __FILE__ and __LINE__ to constructor.
   * @param what The exception message as std::string.
   * @param thing The DebugObject as context.
   */
  #define throw_ExceptionObject( what, thing ) throw dodo::common::Exception( __FILE__, __LINE__, what, thing )

  /**
   * Throws an Exception with errno, passes __FILE__ and __LINE__ to constructor.
   * @param what The exception message as std::string.
   * @param errno The error number.
   */
  #define throw_SystemException( what, errno ) throw dodo::common::SystemException( __FILE__, __LINE__, what, errno )

  /**
   * Throws an Exception with errno, passes __FILE__ and __LINE__ to constructor.
   * @param what The exception message as std::string.
   * @param errno The error number.
   * @param thing The DebugObject as context.
   */
  #define throw_SystemExceptionObject( what, errno, thing ) throw dodo::common::SystemException( __FILE__, __LINE__, what, errno, thing )

};

#endif