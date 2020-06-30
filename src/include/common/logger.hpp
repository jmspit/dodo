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

namespace dodo::common {

  /**
   * A asynchronous Logger interface.
   */
  class Logger {
    public:

      /**
       * The level of a log entry.
       */
      enum class LogLevel {
        Fatal,            /**< The program could not continue. */
        Error,            /**< The program raised an error. Errors are unrecoverable failures. */
        Warning,          /**< The program raised a warning. Warnings are notable events but things are not broken yet. */
        Info,             /**< The program raised an informational message. */
        Debug,            /**< The program produces debug info. */
        Trace,            /**< The program produces trace info. */
        Default = Info,
      };


      /**
       * A LogEntry.
       */
      struct LogEntry {
        /** The time the LogEntry was created. */
        std::time_t created;

        /** The origin of the LogEntry (can but does not have to be a hostname or container name) */
        std::string origin;

        /** The name of the progam that issued the LogEntry. */
        std::string programName;

        /** The TID of the thread that created the LogEntry. */
        long tid;

        /** The log message. */
        std::string logMessage;
      };

    protected:
      /**
       * LogEntry instances > loglevel_ will be ignored.
       */
      LogLevel loglevel_;
  };

}