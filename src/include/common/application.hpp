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

#include <string>

namespace dodo::common {


  /**
   * @brief Red tape wrapper class for applications, from command line to services. The Application class
   *
   *   - initializes and closes the dodo library.
   *   - installs signal handlers, flag when the Application is requested to stop.
   *   - determining the HostType.
   *   - exposing and managing the configuration file
   *   - providing a logging mechanism
   *
   * @code
   * using namespace dodo::common;
   *
   * class MyApp : public common::Application {
   *   public:
   *     MyApp( const StartParameters &param ) : common::Application( param ) {}
   *     virtual int run() { cout << "Hello world!" << endl; return 0; }
   * };

   * int main( int argc, char* argv[], char** envp ) {
   *   try {
   *     MyApp app( { "myapp", "myapp.cnf", argc, argv, envp } );
   *     return app.run();
   *   }
   *   catch ( const std::exception &e ) {
   *     cerr << e.what() << endl;
   *     return 1;
   *   }
   * }
   * @endcode
   */
  class Application {
    public:

      /**
       * The type of host the Application is running on.
       */
      enum class HostType {
        BareMetal,   /**< Bare metal deployment. */
        Docker,      /**< Docker container. */
        VirtualBox,  /**< VirtualBox. */
        VMWare,      /**< VMWare. */
        KVM,         /**< RedHat KVM. */
        GenericVM,   /**< An indeterminate virtual machine. */
      };

      /**
       * Start parameters for the Application.
       */
      struct StartParameters {
        std::string name;     /**< Application name. */
        std::string config;   /**< Configuration file. */
        int argc;             /**< Argument count. */
        char** argv;          /**< Argument array. */
        char** envp;          /**< Environment variables. */
      };

      /**
       * Construct an Application instance.
       * @param param The Application start parameters.
       * @throw dodo::common::Exception when the name is empty, the config file is not empty and invalid.
       */
      Application( const StartParameters &param );

      /**
       * Destructor
       */
      virtual ~Application();


      /**
       * Return the HostType the Application is running on.
       * @return The HostType.
       */
      HostType getHostType() const { return hosttype_; }

      /**
       * Return true when the main thread got a SIGTERM or SIGQUIT.
       * @return true when the main thread got a SIGTERM or SIGQUIT.
       */
      bool hasStopRequest() const { return has_stop_request_; }

      /**
       * Override.
       * @return the return code as returned to the OS (exit code of the process).
       */
      virtual int run() { return 0; }
    private:

      /**
       * Detect the HostType of the host the Application is running on.
       * @return The HostType;
       */
      HostType detectHostType();

      /**
       * Signal handler called by the OS.
       * Depends on application_ to be valid.
       * @param signal the signal received.
       * @return void
       */
      static void signal_handler( int signal );

      /**
       * Install the signal handlers.
       */
      void installSignalHandlers();

      /**
       * Called by signal_handler. Note that this call can be made at any time interrupting the current thread,
       * but no need for synchronization setting has_stop_request_ to true, other threads only read it.
       * @param signal The received signal.
       */
      void onSignal( int signal );

      /**
       * Singleton Application object.
       */
      static Application* application_;

      /**
       * The application name
       */
      std::string app_name_;

      /**
       * The configuration file name
       */
      std::string config_file_;

      /**
       * The HostType, detected once in the Application constructor.
       */
      HostType hosttype_;

      /**
       * True when the Application main pid got a signal to stop (SIGTERM).
       */
      bool has_stop_request_;
  };

}

#endif