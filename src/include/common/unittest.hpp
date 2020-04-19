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

  /**
   * Unit test assitence class.
   */
  class UnitTest {
    public:

      /**
       * Create a test
       * @param name The name of the test.
       * @param description The description of the test.
       * @param out The ostream to write to.
       */
      UnitTest( const string &name, const string &description, ostream *out );

      /**
       * Destructor.
       */
      virtual ~UnitTest() {};

      /**
       * Run the test.
       * @return True if the test passed.
       */
      bool run();

    protected:

      /**
       * Virtual to implement the test.
       */
      virtual void doRun() = 0;

      /**
       * Write the subtest result.
       * @param name The name of the test.
       * @param description The description of the test.
       * @param passed True if the test passed.
       * @return The value of parameter passed.
       */
      bool writeSubTestResult( const string &name, const string& description, bool passed );

    private:

      /** Write the header. */
      void writeUnitTestHeader();

      /** Write the summary. */
      void writeUnitTestSummary();

      /** The test name. */
      string name_;

      /** The test description. */
      string description_;

      /** The output destination. */
      ostream *out_;

      /** The total number of tests. */
      size_t total_;

      /** The total number of failed tests. */
      size_t failed_;
  };

}

#endif