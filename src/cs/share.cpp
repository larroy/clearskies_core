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
#include "utils.hpp"
#include <iostream>

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
    , m_psk_untrusted()
    , m_pkc_rw()
    , m_pkc_ro()
{
    bfs::path share_path_(share_path);
    if (! bfs::exists(share_path_))
        throw std::runtime_error(fs("Share::Share error: " << share_path_ << " doesn't exist"));

    if (! bfs::is_directory(share_path_))
        throw std::runtime_error(fs("Share::Share error: " << share_path_ << " not a directory"));

    initialize_tables();
}

void Share::initialize_tables()
{
    sqlite3pp::command create_files_table(m_db, R"#(CREATE TABLE IF NOT EXISTS files (
        path TEXT PRIMARY KEY,
        utime TEXT,
        mtime TEXT,
        size INTEGER,
        mode INTEGER,
        sha256 TEXT,
        deleted INTEGER DEFAULT 0)
    )#");
}


std::unique_ptr<File> Share::get_file_info(const std::string& path)
{
    auto result = make_unique<File>();
    sqlite3pp::query file_q(m_db, "SELECT path, utime, mtime, size, mode, sha256, deleted FROM files WHERE path = :path");
    file_q.bind(":path", path);

    bool found = false;
    for (auto i = file_q.begin(); i != file_q.end(); ++i)
    {
        assert(! found);
        assert(i->get<std::string>(0) == path);
        result->path = path;
        result->utime = i->get<std::string>(1);
        result->mtime = i->get<std::string>(2);
        result->size = i->get<long long int>(3);
        result->mode = i->get<int>(4);
        result->sha256 = i->get<std::string>(5);
        result->deleted = i->get<int>(6) != 0;
        found = true;
    }
    return move(result);
}

void Share::set_file_info(const File& f)
{
    sqlite3pp::command file_i(m_db, "INSERT INTO files (path, utime, mtime, size, mode, sha256, deleted) VALUES (?,?,?,?,?,?,?)");
    file_i.bind(1, f.path);
    file_i.bind(2, f.utime);
    file_i.bind(3, f.mtime);
    file_i.bind(4, static_cast<long long int>(f.size));
    file_i.bind(5, static_cast<int>(f.mode));
    file_i.bind(6, f.sha256);
    file_i.bind(7, static_cast<int>(f.deleted));

    file_i.exec();
}

void Share::scan()
{
    // FIXME create thread with scan_thread code.
    // when it finishes we get some information on how much is there to checksum for progress
    // then we checksum, when both steps are finished we need to somehow notify the main theads
    // protocolstate so it can send updates etc. We can do this by sending commands to a control
    // pipe or with other mechanism TBD
}

void Share::scan_thread()
{
    bfs::recursive_directory_iterator it(m_path);
    bfs::recursive_directory_iterator end;
    for ( ; it != end; ++it)
    {
        const auto& dentry = *it;
        if (dentry.status().type() == bfs::regular_file)
        {
            File f;
            // we get the path relative to the share
            bfs::path fpath = get_tail(dentry.path(), it.level() + 1);
            assert(fpath.is_relative());
            f.path = fpath.string();
            // FIXME convert to iso time
            // FIXME utime
            f.mtime = fs(bfs::last_write_time(dentry.path()));
            f.size = bfs::file_size(dentry.path());
            f.mode = dentry.status().permissions();
            scan_file(move(f));

#if 0
            cout << f.path << endl;
            cout << f.mtime << endl;
            cout << oct << f.mode << dec << endl;
            cout << f.size << endl;
            cout << endl;
#endif
        }
    }
}


void Share::scan_file(File&& file)
{
    /* Compare file mtime, if file is new || size || mtime don't match saved mark for checksum
     *
     */

    //sqlite3pp::query file_q(m_db, "");
}


bfs::path get_tail(const bfs::path& path, size_t tail)
{
    auto pi = path.begin();
    auto pe = path.end();
    assert(distance(pi, pe) >= (i64)tail);
    pi = pe;
    while (tail > 0)
    {
        --pi;
        --tail;
    }
    bfs::path result;
    while (pi != pe)
        result /= *pi++;
    return result;
}


} // end ns
} // end ns
