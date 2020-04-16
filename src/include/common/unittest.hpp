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
 * @file puts.hpp
 * Defines the dodo::common::Puts class.
 */

#ifndef common_unittest_hpp
#define common_unittest_hpp

#include <iostream>
#include <string>

using namespace std;

namespace dodo::common {

  class UnitTest {
    public:
      UnitTest( const string &name, const string &description, ostream *out );
      virtual ~UnitTest() {};
      bool run();
    protected:
      virtual void doRun() = 0;
      bool writeSubTestResult( const string &name, const string& description, bool passed );
    private:
      void writeUnitTestHeader();
      void writeUnitTestSummary();
      string name_;
      string description_;
      ostream *out_;
      size_t total_;
      size_t failed_;
  };

}

#endif