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
 * @file socket.cpp
 * Implements the dodo::network::Socket class.
 */

#include "common/unittest.hpp"

namespace dodo::common {

  UnitTest::UnitTest( const string& name, const string& description, ostream *out ) :
    name_(name),
    description_(description),
    out_(out),
    total_(0),
    failed_(0) {
  }

  bool UnitTest::run() {
    writeUnitTestHeader();
    doRun();
    writeUnitTestSummary();
    return failed_ == 0;
  }

  void UnitTest::writeUnitTestHeader() {
    *out_ << string( 80, '=' ) << endl;
    *out_ << name_ << " - " << description_ << endl;
    *out_ << string( 80, '=' ) << endl;
    *out_ << endl;
  }

  void UnitTest::writeUnitTestSummary() {
    *out_ << string( 80, '+' ) << endl;
    *out_ << failed_ << " failed out of " << total_ << " tests." << endl;
    *out_ << endl;
  }

  bool UnitTest::writeSubTestResult( const string &name, const string& description, bool passed ) {
    total_++;
    if ( !passed ) failed_++;
    *out_ << string( 80, '-' ) << endl;
    *out_ << name << " - " << description << " : " << (passed?"passed":"failure") << endl;
    *out_ << string( 80, '-' ) << endl;
    *out_ << endl;
    return passed;
  }

}