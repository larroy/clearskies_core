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
#if __linux__ or __unix
#include <unistd.h>
#endif
#include "utils.hpp"
#include <functional>

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
    , m_shares()
    , m_daemon()
    , m_loop()
    , m_tcp_listen_conn(m_loop)
{
}

Daemon::~Daemon()
{
    stop();
}


void Daemon::attach_share(const std::string& share_path, const std::string& dbpath)
{
    if (dbpath.empty())
    {
        share::Share share(share_path);
        string share_id = share.m_share_id;
        m_shares.emplace(move(share_id), move(share));
    }
    else
    {
        share::Share share(share_path, dbpath);
        string share_id = share.m_share_id;
        m_shares.emplace(move(share_id), move(share));
    }
}

void Daemon::daemonize()
{
// Sqlite dbs shouldn't be carried open across a fork
#ifdef __unix
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
    m_tcp_listen_conn.listen(std::bind(&Daemon::on_connect, this, placeholders::_1));
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


void Daemon::on_connect(uvpp::error error)
{

}

} // end ns
} // end ns

