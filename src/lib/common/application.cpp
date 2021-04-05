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
 * @file application.cpp
 * Implements the dodo::common::Application class..
 */

#include <common/application.hpp>
#include <common/logger.hpp>
#include <dodo.hpp>

#include <csignal>

namespace dodo::common {

  Application* Application::application_ = nullptr;

  Application::Application( const StartParameters &param ) {
    application_ = this;
    has_stop_request_ = false;
    dodo::initLibrary();
    installSignalHandlers();
    hosttype_ = detectHostType();
    Config* config = Config::initialize( param.config );
    logger_ = Logger::initialize( *config );
    logger_->log( Logger::LogLevel::Info, Puts() << config->getAppName() << " started" );
  }

  Application::~Application() {
    if ( Logger::logger_ ) delete Logger::logger_;
    if ( Config::config_ ) delete Config::config_;
    dodo::closeLibrary();
  }

  void Application::signal_handler( int signal ) {
    application_->onSignal( signal );
  }

  void Application::installSignalHandlers() {
    std::signal( SIGINT, signal_handler );
    std::signal( SIGQUIT, signal_handler );
    std::signal( SIGTERM, signal_handler );
  }

  void Application::onSignal( int signal ) {
    log_Warning( "caught signal " << signal );
    switch ( signal ) {
      case SIGINT:
      case SIGQUIT:
      case SIGTERM:
        has_stop_request_ = true;
        break;
      default: return;
    }
  }


  Application::HostType Application::detectHostType() {
    const std::string initcgroup = "/proc/1/cgroup";
    if ( common::fileReadAccess( initcgroup ) ) {
      auto res = fileReadStrings( initcgroup, std::regex("^0::/docker$") );
      if ( res.size() == 1 ) return HostType::Docker;
    }

    const std::string product_name = "/sys/class/dmi/id/product_name";
    if ( common::fileReadAccess( product_name ) ) {
      auto res = fileReadStrings( product_name, std::regex("^VirtualBox$") );
      if ( res.size() == 1 ) return HostType::VirtualBox;

      res = fileReadStrings( product_name, std::regex("^VMware Virtual Platform$") );
      if ( res.size() == 1 ) return HostType::VMWare;

      res = fileReadStrings( product_name, std::regex("^KVM$") );
      if ( res.size() == 1 ) return HostType::KVM;
    }

    const std::string cpuinfo = "/proc/cpuinfo";
    if ( common::fileReadAccess( cpuinfo ) ) {
      auto res = fileReadStrings( cpuinfo, std::regex("^flags.*:.*hypervisor") );
      if ( res.size() > 0 ) return HostType::GenericVM;
    }

    return HostType::BareMetal;
  }

}