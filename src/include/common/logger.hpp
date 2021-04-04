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
 * @file logger.hpp
 * Defines the dodo::network::Logger class.
 */

#include <threads/mutex.hpp>

#include <fstream>
#include <map>
#include <functional>

#ifndef common_logger_hpp
#define common_logger_hpp

namespace dodo::common {

  class Config;

  /**
   * A Logger interface. Calls to log( LogLevel level, const std::string message ) are serialized internally
   * and thus thread-safe.
   */
  class Logger {
    public:

      /**
       * The level of a log entry.
       */
      enum class LogLevel {
        Fatal,            /**< The program could not continue. */
        Error,            /**< The program signaled an error. */
        Warning,          /**< The program signaled a warning. */
        Info,             /**< The program signaled an informational message. */
        Statistics,       /**< The program signaled runtime statistics. */
        Debug,            /**< The program produced debug info. */
        Trace,            /**< The program produced trace info. */
      };

      /**
       * Destination flags.
       */
      enum Destination {
        Console = 1,
        File = 2,
        Syslog = 4,
      };

      /**
       * File Logging parameters.
       */
      struct FileParams {
        /** The directory to write the log files in. */
        std::string directory;
        /** The maximum size of a logfile. */
        size_t max_size_mib = 10;
        /** The maximum number of files to keep besides the active log file. */
        size_t max_file_trail = 4;
        /** the filename of the active log */
        std::string active_log;
        /** the ofstream of the active log. */
        std::ofstream file;
        /** current file size */
        size_t filesize = 0;
      };

      /**
       * syslog logging parameters.
       */
      struct SyslogParams {
        /** The syslog facility to use. */
        int facility;
      };

      /**
       * Log a Fatal log entry. These are always logged as Fatal is the lowest LogLevel.
       * Calling is thread-safe.
       * @param message The fatal log message.
       */
      void fatal( const std::string &message );

      /**
       * Log a Error log entry. These are only logged when LogLevel >= Error.
       * Calling is thread-safe.
       * @param message The Error log message.
       */
      void error( const std::string &message );

      /**
       * Log a Warning log entry. These are only logged when LogLevel >= Warning.
       * Calling is thread-safe.
       * @param message The Warning log message.
       */
      void warning( const std::string &message );

      /**
       * Log a Info log entry. These are only logged when LogLevel >= Info.
       * Calling is thread-safe.
       * @param message The Info log message.
       */
      void info( const std::string &message );

      /**
       * Log a Statistics log entry. These are only logged when LogLevel >= Statistics.
       * Calling is thread-safe.
       * @param message The Statistics log message.
       */
      void statistics( const std::string &message );

      /**
       * Log a Debug log entry. These are only logged when LogLevel >= Debug.
       * Calling is thread-safe.
       * @param message The Debug log message.
       */
      void debug( const std::string &message );

      /**
       * Log a Trace log entry. These are only logged when LogLevel >= Trace.
       * Calling is thread-safe.
       * @param message The Trace log message.
       */
      void trace( const std::string &message );

      /**
       * Initialize the Logger singleton.
       * @param config The Config to use.
       * @return A pointer to the singleton.
       */
      static Logger* initialize( const Config& config );


      /**
       * Get the Logger singleton - initialize() must have been called first.
       * @param config The Config to use.
       * @return A pointer to the singleton.
       */
      static Logger* getLogger();

      /**
       * Return the LogLevel as a string (as used in console and file).
       * @param level The LogLevel.
       * @param acronym If true, return a uppercase character triplet for use in log files.
       * @return A string representation of the LogLevel.
       */
      static std::string LogLevelAsString( LogLevel level, bool acronym );

      /**
       * Return the string as a LogLevel. Note this will default to Info if string is not recognized.
       * @param slevel The LogLevel as a string.
       * @return The LogLevel.
       */
      static LogLevel StringAsLogLevel( const std::string slevel );

    protected:

      /**
       * Construct against a config.
       * @param config The configuration to read from.
       */
      Logger( const Config& config );

      /**
       * Destructor.
       */
      virtual ~Logger();

      /**
       * Log a log entry. The entry is only written when level <= the loglevel specified in the Config,
       * and silently ignored otherwise. Calling is thread-safe, but the lock is not acquired at all if
       * the entry is ignored by level.
       * @param level The logLevel of the messsage.
       * @param message The log message.
       */
      void log( LogLevel level, const std::string &message );

      /**
       * Format a LogLine.
       * @param level The logLevel of the messsage.
       * @param message The log message.
       * @return A formatted log line.
       */
      std::string formatMessage( LogLevel level, const std::string &message );


      /**
       * Get FileParams from the Config into file_params_.
       * @param config The Config to get the FileParams from.
       */
      void getFileParams( const Config& config );

      /**
       * Check and rotate the log file if it exceeds the size limit,
       * delete older logs if needed.
       */
      void checkRotate();

      /**
       * Map a LogLevel to a syslog level.
       * @param level The LogLevel to map.
       * @return The mapped syslog level.
       */
      int mapLeveltoSyslog( LogLevel level );

      /**
       * threads::Mutex to serialize log writing.
       */
      static threads::Mutex mutex_;

      /**
       * The singleton.
       */
      static Logger* logger_;

      /**
       * Destinations as bit flags
       */
      uint8_t destinations_;

      /**
       * Map Destination to LogLevel.
       */
      std::map<uint8_t,LogLevel> levels_;

      /**
       * The hostname cached in the constructor.
       */
      std::string hostname_;

      /**
       * Parameters for the file Destination.
       */
      FileParams file_params_;

      /**
       * Parameters for the syslog Destination.
       */
      SyslogParams syslog_params_;

      /**
       * Track when rotation was last checked.
       */
      size_t rotate_throttle_counter_;

    /**
     * Application::~Application() will destruct this singleton
     */
    friend class Application;

  };

  /**
   * Macro to log Fatal.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #define log_Fatal( what ) common::Logger::getLogger()->fatal( dodo::common::Puts() << what )

  /**
   * Macro to log Error.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #define log_Error( what ) common::Logger::getLogger()->error( dodo::common::Puts() << what )

  /**
   * Macro to log Warning.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #define log_Warning( what ) common::Logger::getLogger()->warning( dodo::common::Puts() << what )

  /**
   * Macro to log Info.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #define log_Info( what ) common::Logger::getLogger()->info( dodo::common::Puts() << what )

  /**
   * Macro to log Statistics.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #define log_Statistics( what ) common::Logger::getLogger()->statistics( dodo::common::Puts() << what )

  /**
   * Macro to log Debug.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #ifndef NDEBUG
  #define log_Debug( what ) common::Logger::getLogger()->debug( dodo::common::Puts() << what )
  #else
  #define log_Debug( what )
  #endif

  /**
   * Macro to log Trace.
   * @param what The info to log. As the argument is fed to a dodo::common::Puts() it can be a << pipe-lined expression.
   */
  #ifndef NDEBUG
  #define log_Trace( what ) common::Logger::getLogger()->trace( dodo::common::Puts() << what )
  #else
  #define log_Trace( what )
  #endif

}

#endif