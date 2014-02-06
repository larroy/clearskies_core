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
#include "share.hpp"
#include "boost_fs_fwd.hpp"
#include "utils.hpp"

using namespace std;

namespace cs
{
namespace share
{

Share::Share(const std::string& share_path, const std::string& dbpath):
      m_path(share_path)
    , m_db(dbpath.c_str())
    , m_db_path(dbpath)
    , m_share_id()
    , m_peer_id()
    , m_psk_rw()
    , m_psk_ro()
    , m_psk_ut()
    , m_pkc_rw()
    , m_pkc_ro()
{
    bfs::path share_path_(share_path);
    if (! bfs::exists(share_path_))
        throw std::runtime_error(fs("Share::Share error: " << share_path_ << " doesn't exist"));

    if (! bfs::is_directory(share_path_))
        throw std::runtime_error(fs("Share::Share error: " << share_path_ << " not a directory"));
}

void Share::scan()
{
}

void Share::scan_thread()
{
    bfs::recursive_directory_iterator it(m_path); 
    bfs::recursive_directory_iterator end;
    for ( ; it != end; ++it)
    {
        cout << it->path().string() << endl;
    }
}


} // end ns
} // end ns
