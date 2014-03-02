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
#include "server.hpp"

using namespace std;

namespace cs
{

namespace server
{

void Server::attach_share(const std::string& share_path, const std::string& dbpath)
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

} // end ns
} // end ns
