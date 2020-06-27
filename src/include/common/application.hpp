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
 * @file application.hpp
 * Defines the dodo::common::Config class.
 */

#ifndef common_application_hpp
#define common_application_hpp

namespace dodo::common {


  /**
   * @brief To use as program entry point.
   *
   * @code
   * using namespace dodo::common;
   *
   * class MyApp : public Application {
   *   public:
   *     virtual int run() { cout << "Hello world!" << endl; return 0; }
   * }
   *
   * int main( int argc, char* argv[], char** envp ) {
   *   try {
   *     MyApp app( "myapp", "myapp.cnf", argc, argv, envp );
   *     return app.run();
   *   }
   *   catch ( const std::exception &e ) {
   *     cerr << e.what() << endl;
   *     return 2;
   *   }
   * }
   * @endcode
   */
  class Application {
    public:
      /**
       * Construct an Application instance.
       * @param name The name of the application (used, for example, in logging).
       * @param config The location of the configuration file. Ignored if an empty string. If not empty, the config
       *                file must exist and have a valid format.
       * @param argc The argument count as given to main.
       * @param argv The argument values as given to main.
       * @param envp The environment pointer as given to main.
       * @throw dodo::common::Exception when the name is empty, the config file is not empty and invalid.
       */
      Application( const std::string name,
                   const std::string config,
                   int argc,
                   char* argv,
                   char** envp ) {}

      /**
       * Destructor
       */
      ~Application();

      /**
       * Override.
       * @return the return code as returned to the OS (exit code of the process).
       */
      virtual int run() { return 0; }
    protected:

      /**
       * The application name
       */
      std::string app_name_;

      /**
       * The configuration file name
       */
      std::string config_file_;
  };

}

#endif