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

#pragma once
#include "config.hpp"
#include "server.hpp"
#include "uvpp/uvpp.hpp"
#include <string>
#include <unordered_map>

namespace cs
{
namespace daemon
{

class TCPConnection: public server::Connection
{
public:
    TCPConnection(
        const ServerInfo& server_info,
        const std::map<std::string, share::Share>& shares,
        uvpp::loop& loop
    ):
        server::Connection(server_info, shares)
        , r_loop(loop)
        , m_tcp_conn(loop)
    {

    }
    uvpp::loop& r_loop;
    uvpp::Tcp m_tcp_conn;
};




class Daemon: public server::Server
{
public:
    Daemon();
    ~Daemon();

    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;

    void daemonize();
    void start();
    void stop();
    void set_port(i16 port);

private:
    void on_tcp_connect(uvpp::error error);

    i16 m_port;
    bool m_running;
    bool m_daemon;
    uvpp::loop m_loop;
    uvpp::Tcp m_tcp_listen_conn;
};

} // end ns
} // end ns

