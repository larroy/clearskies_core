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
#include "int_types.h"
namespace sha2
{
#include "sha2/sha2.h"
}

using namespace std;

namespace
{

void file_from_row(cs::File& file, const sqlite3pp::query::rows& row)
{
    file.path = row.get<const char*>(0);
    file.utime = row.get<const char*>(1);
    file.mtime = row.get<const char*>(2);
    file.size = row.get<long long int>(3);
    file.mode = row.get<int>(4);
    file.sha256 = row.get<const char*>(5);
    file.deleted = row.get<int>(6) != 0;
}

/**
 * will open the file and calculate sha256, @throws runtime_error in IO error
 */

std::string sha256(const bfs::path& p)
{
    using namespace sha2;
    string result(SHA256_DIGEST_STRING_LENGTH, 0);
    assert(p.is_absolute());

    SHA256_CTX  c256;
    SHA256_Init(&c256);

    array<char, 65536> rbuff;

    bfs::ifstream is(p, ios_base::in | ios_base::binary);
    is.exceptions(ios::badbit);

    while(is)
    {
        is.read(rbuff.data(), rbuff.size());
        SHA256_Update(&c256, (const cs::u8*) rbuff.data(), is.gcount());
    }

    SHA256_End(&c256, &result[0]);
    result.resize(result.size() - 1);
    return result;
}


} // end anon ns

namespace cs
{
namespace share
{

Share::Share_iterator::Share_iterator():
    m_query()
    , m_query_it()
    , m_file()
    , m_file_set()
{}

Share::Share_iterator::Share_iterator(Share& share):
    m_query(make_unique<sqlite3pp::query>(share.m_db, "SELECT path, utime, mtime, size, mode, sha256, deleted FROM files ORDER BY path"))
    , m_query_it(m_query->begin())
    , m_file()
    , m_file_set()
{
}

void Share::Share_iterator::increment()
{
    ++m_query_it;
    m_file_set = false;
}

File& Share::Share_iterator::dereference() const
{
    if (! m_file_set)
    {
        file_from_row(m_file, *m_query_it);
        m_file_set = true;
    }
    return m_file;
}


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
    // avoid excessive IO
    vector<sqlite3pp::command> performance_adjusts;
    performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA synchronous = 0"));
    performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA cache_size = 1000"));
    performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA fullfsync = 0"));
    // performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA journal_mode = OFF"));

    for (sqlite3pp::command& cmd: performance_adjusts)
        cmd.exec();

    sqlite3pp::command create_files_table(m_db, R"#(CREATE TABLE IF NOT EXISTS files (
        path TEXT PRIMARY KEY,
        utime TEXT,
        mtime TEXT,
        size INTEGER,
        mode INTEGER,
        sha256 TEXT,
        deleted INTEGER DEFAULT 0)
    )#");
    create_files_table.exec();
}


void Share::scan()
{
    // FIXME create thread with scan_thread code.
    // when it finishes we get some information on how much is there to checksum for progress
    // then we checksum, when both steps are finished we need to somehow notify the main theads
    // protocolstate so it can send updates etc. We can do this by sending commands to a control
    // pipe or with other mechanism TBD
    // as suggested by Jewel: http://nikhilm.github.io/uvbook/threads.html uv_async_send, etc.
}



std::unique_ptr<File> Share::get_file_info(const std::string& path)
{
    unique_ptr<File> result;
    sqlite3pp::query file_q(m_db, "SELECT path, utime, mtime, size, mode, sha256, deleted FROM files WHERE path = :path");
    file_q.bind(":path", path);

    bool found = false;
    for (const auto& row: file_q)
    {
        assert(! found); // path must be unique, it's pk
        result = make_unique<File>();
        assert(row.get<std::string>(0) == path);
        file_from_row(*result, row);
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
            // FIXME utime
            f.mtime = utils::isotime(bfs::last_write_time(dentry.path()));
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

void Share::checksum_thread()
{

    sqlite3pp::query file_q(m_db, "SELECT path, utime, mtime, size, mode, sha256, deleted FROM files WHERE sha256 = '' ORDER BY path");
    for (const auto& row: file_q)
    {
        File file;
        file_from_row(file, row);

        bfs::path fullpath = m_path / bfs::path(file.path);
        file.sha256 = sha256(fullpath);

        sqlite3pp::command update_hash(m_db, "UPDATE files SET sha256 = :sha256 WHERE path = :path");
        update_hash.bind(":sha256", file.sha256);
        update_hash.bind(":path", file.path);
        update_hash.exec();
    }
    // TODO: progress update
}

void Share::scan_file(File&& file_found)
{
    // TODO: add bytes to checksum for stats
    unique_ptr<File> file_prev = get_file_info(file_found.path);
    if (file_prev)
    {
        if (file_found.mtime != file_prev->mtime
            || file_found.size != file_prev->size)
        {
            set_file_info(file_found);
        }
    }
    else
        set_file_info(file_found);
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
