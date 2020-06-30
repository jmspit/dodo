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

#include "common/application.hpp"
#include "dodo.hpp"

#include <csignal>

namespace dodo::common {

  Application* Application::application_ = nullptr;

  Application::Application( const StartParameters &param ) {
    application_ = this;
    app_name_ = param.name;
    has_stop_request_ = false;
    dodo::initLibrary();
    installSignalHandlers();
    hosttype_ = detectHostType();
    Config::initialize( param.config );
  }

  Application::~Application() {
    dodo::closeLibrary();
  }

  void Application::signal_handler( int signal ) {
    application_->onSignal( signal );
  }

  void Application::installSignalHandlers() {
    for ( int s = 1; s <= SIGRTMAX; s++ ) {
      std::signal( s, signal_handler );
    }
  }

  void Application::onSignal( int signal ) {
    //Logger::getLogger().log( Logger::lvWarning, common::Puts() << "caught signal " << signal );
    cout << "received signal " << signal << endl;
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
      std::vector res = fileReadStrings( initcgroup, std::regex("^0::/docker$") );
      if ( res.size() == 1 ) return HostType::Docker;
    }

    const std::string product_name = "/sys/class/dmi/id/product_name";
    if ( common::fileReadAccess( product_name ) ) {
      std::vector res = fileReadStrings( product_name, std::regex("^VirtualBox$") );
      if ( res.size() == 1 ) return HostType::VirtualBox;
    }

    if ( common::fileReadAccess( product_name ) ) {
      std::vector res = fileReadStrings( product_name, std::regex("^VMware Virtual Platform$") );
      if ( res.size() == 1 ) return HostType::VMWare;
    }

    if ( common::fileReadAccess( product_name ) ) {
      std::vector res = fileReadStrings( product_name, std::regex("^KVM$") );
      if ( res.size() == 1 ) return HostType::KVM;
    }

    const std::string cpuinfo = "/proc/cpuinfo";
    if ( common::fileReadAccess( cpuinfo ) ) {
      std::vector res = fileReadStrings( cpuinfo, std::regex("^flags.*:.*hypervisor") );
      if ( res.size() > 0 ) return HostType::GenericVM;
    }

    return HostType::BareMetal;
  }

}