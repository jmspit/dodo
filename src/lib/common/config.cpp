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
 * @file config.cpp
 * Implements the dodo::common::Config class..
 */

#include <fstream>

#include "common/config.hpp"
#include "common/exception.hpp"

namespace dodo::common {

  Config* Config::config_ = nullptr;

  std::string Config::path_ = "";

  Config::Config( const std::string path ) {
    path_ = path;
    readConfig();
  }

  Config* Config::initialize( const std::string path ) {
    if ( config_ ) throw_Exception( "calling initialize on an existing Config singleton" );
    config_ = new Config( path );
    return config_;
  }

  Config* Config::getConfig() {
    if ( config_ == nullptr ) throw_Exception( "null singleton - call initialize before getConfig" );
    return config_;
  }

  void Config::readConfig() {
    yaml_ = YAML::LoadFile(path_);
  }

  void Config::writeConfig() {
    std::ofstream fout(path_);
    fout << yaml_;
  }

}