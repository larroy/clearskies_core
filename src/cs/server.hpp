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
#include "share.hpp"
#include "serverinfo.hpp"
#include "protocolstate.hpp"
#include "clearskiesprotocol.hpp"
#include <string>
#include <map>

namespace cs
{

namespace server
{


/**
 * Base class for connections to peers
 */
class Connection
{
public:
    Connection(
        const ServerInfo& server_info,
        const std::map<std::string, share::Share>& shares
    ):
        m_cs_protocol(server_info, shares)
    {}

    virtual ~Connection() = default;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = default;
    Connection& operator=(Connection&&) = default;

    cs::protocol::ClearSkiesProtocol m_cs_protocol;
};



/**
 * Base class that knows about some shares and where to they map in the filesystem
 */
class Server 
{
public:
    Server(): 
        m_shares()
        , m_connections()
        , m_server_info()
    {}

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;

    /**
     * We attach a path and a database holding the configuration for this share. If @param dbpath is
     * not provided, the share is transient, stored in memory.
     * @returns share ID
     */
    std::string attach_share(const std::string& share_path, const std::string& dbpath = std::string());

    /// @returns a reference to the given share, @throws runtime_error if share is not known 
    share::Share& share(const std::string& share_id);

    /// @returns the list of known share_ids
    std::vector<std::string> shares() const;

protected:
    /// share id to @sa share::Share, the share knows the path
    std::map<std::string, share::Share> m_shares;
    /// connection identifier to Connection
    std::map<std::string, std::unique_ptr<Connection>> m_connections;

public:
    ServerInfo m_server_info;

};



} // end ns
} // end ns

