/*
 *  This file is part of clearskies_core file synchronization program
 *  Copyright (C) 2014 Pedro Larroy

 *  clearskies_core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  clearskies_core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public License
 *  along with clearskies_core.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "daemon.hpp"

namespace cs
{
namespace daemon
{

Daemon::Daemon(conf::Conf& conf):
      r_conf(conf)
    , m_port(r_conf.daemon_port())
    , m_running()
    , m_shares()
{
}

Daemon::~Daemon()
{
    stop();
}


void Daemon::attach_share(const std::string& share_path, const std::string& dbpath)
{
    if (dbpath.empty())
        m_shares.emplace_back(share_path);
    else
        m_shares.emplace_back(share_path, dbpath);
}


void Daemon::start()
{
    m_running = true;
}


void Daemon::stop()
{

    m_running = false;
}


void Daemon::set_port(i16 port)
{
    if (m_running)
        throw std::runtime_error("Daemon::set_port can't change while running");

    m_port = port;
}


} // end ns
} // end ns

