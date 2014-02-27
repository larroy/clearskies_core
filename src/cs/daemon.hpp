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
#include <string>
#include <unordered_map>
#include "share.hpp"
#include "config.hpp"
#include "uvpp/uvpp.hpp"
#include "clearskiesprotocol.hpp"

namespace cs
{
namespace daemon
{

namespace internal
{

class TCPconnectionState
{
public:
    TCPconnectionState(uvpp::loop& loop):
        r_loop(loop)
        , m_tcp_conn(loop)
    {

    }
    uvpp::loop& r_loop;
    uvpp::Tcp m_tcp_conn;
    cs::protocol::ClearSkiesProtocol m_protocol;
};



} // end ns

class Daemon
{
public:
    Daemon();
    ~Daemon();

    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;

    void attach_share(const std::string& share_path, const std::string& dbpath = std::string());
    void daemonize();
    void start();
    void stop();
    void set_port(i16 port);

private:
    void on_tcp_connect(uvpp::error error);


    i16 m_port;
    bool m_running;
    std::unordered_map<std::string, share::Share> m_shares;
    bool m_daemon;
    uvpp::loop m_loop;
    uvpp::Tcp m_tcp_listen_conn;
    std::vector<internal::TCPconnectionState> m_connections;
};

} // end ns
} // end ns

