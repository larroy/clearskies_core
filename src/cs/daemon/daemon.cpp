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
#include "protocolstate.hpp"
#include "utils.hpp"
#include <functional>
#include <iostream>

#ifdef CS_PLATFORM_UNIX
#include <unistd.h>
#endif

using namespace std;

namespace
{


} // end ns

namespace cs
{
namespace daemon
{

Daemon::Daemon():
      m_port(0)
    , m_running()
    , m_daemon()
    , m_loop()
    , m_tcp_listen_conn(m_loop)
{
}

Daemon::~Daemon()
{
    stop();
}


void Daemon::daemonize()
{
// Sqlite dbs shouldn't be carried open across a fork
#ifdef CS_PLATFORM_UNIX
    int fd = -1;

    if (fork() != 0) exit(0); /* parent exits */
    setsid(); /* create a new session */
    if(chdir("/") < 0)
        throw runtime_error("ERROR: Daemon::daemonize chdir(\"/\") failed");
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
            close(fd);
    }
    m_daemon = true;
#endif
}

void Daemon::start()
{
    m_tcp_listen_conn.bind6("::", m_port);
    m_tcp_listen_conn.listen(std::bind(&Daemon::on_tcp_connect, this, placeholders::_1));
    m_running = true;
    m_loop.run();
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


void Daemon::on_tcp_connect(uvpp::error error)
{
    auto tcp_conn_ptr = make_unique<TCPConnection>(m_server_info, m_shares, m_loop);
    TCPConnection& tcp_conn = *tcp_conn_ptr;
    m_tcp_listen_conn.accept(tcp_conn.m_tcp_conn);
    protocol::ProtocolState& pstate = tcp_conn.m_cs_protocol;

    string peer_ip;
    int port;
    bool ip4;
    const bool getpeername_ok = tcp_conn.m_tcp_conn.getpeername(ip4, peer_ip, port);
    assert(getpeername_ok);
    (void)getpeername_ok;
    ostringstream os;
    os << "tcp://" << peer_ip << ":" << port;
    string peer_ = os.str();
    //string peer_ = fs("tcp://" << peer_ip << ":" << port); // will go out of scope

    auto res = m_connections.emplace(piecewise_construct,
        forward_as_tuple(move(peer_)),
        forward_as_tuple(move(tcp_conn_ptr)));
    assert(res.second);
    const string& peer = res.first->first; // reference that will stay valid

    // we need to be very careful about capturing objects that might go out of scope. The Connection
    // is owned by Server::m_connections, so remains valid.
    // tcp_conn and p_state are owned by the Server. (m_connections)

    auto close_cb = [this, peer]() {
        m_connections.erase(peer);
    };

    // write finished callback
    auto write_cb = [&](uvpp::error) {
        if (! error)
            pstate.on_write_finished();
        else
        {
            cerr << "TCP client write error: " << peer << endl;
            tcp_conn.m_tcp_conn.close(close_cb);
        }
    };

    // function to be called when the the protocol needs to write
    auto do_write = [&](const char* buff, size_t sz) {
        tcp_conn.m_tcp_conn.write(buff, sz, write_cb);
    };

    auto read_cb = [&](const char* buff, ssize_t len) {
        if (len < 0)
        {
            cerr << "TCP client read error: " << peer << endl;
            tcp_conn.m_tcp_conn.close(close_cb);
        }
        else
            pstate.input(buff, static_cast<size_t>(len));
    };

    // set function to call when the protocol has data to write
    pstate.set_write_fun(do_write);

    tcp_conn.m_tcp_conn.read_start(read_cb);
}

} // end ns
} // end ns

