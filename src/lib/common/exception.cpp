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
 * @file exception.cpp
 * Implement exceptions.
 */

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