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

void mfile_from_row(MFile& file, const sqlite3pp::query::rows& row)
{
    file.path = row.get<const char*>(0);
    file.mtime = row.get<const char*>(1);
    file.size = row.get<long long int>(2);
    file.mode = row.get<int>(3);
    file.sha256 = row.get<const char*>(4);
    file.deleted = row.get<int>(5) != 0;
    file.last_changed_rev = row.get<long long int>(6);
    file.scan_found = row.get<int>(7) != 0;
    file.updated = row.get<int>(8) != 0;
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

    array<char, > rbuff;

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
    m_query(make_unique<sqlite3pp::query>(share.m_db, "SELECT path, mtime, size, mode, sha256, deleted FROM files ORDER BY path"))
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

MFile& Share::Share_iterator::dereference() const
{
    if (! m_file_set)
    {
        mfile_from_row(m_file, *m_query_it);
        m_file_set = true;
    }
    return m_file;
}


Share::Share(const std::string& share_path, const std::string& dbpath):
      m_path(share_path)
    , m_revision()
    , m_db(dbpath.c_str())
    , m_db_path(dbpath)
    , m_scan_in_progress()
    , m_scan_batch_sz(256)
    , m_scan_it()
    , m_cksum_block_sz(65536)
    , m_cksum_batch_sz(8)
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

    //
    // FILES
    //
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS files (
        path TEXT PRIMARY KEY,
        mtime TEXT,
        size INTEGER,
        mode INTEGER,
        scan_found INTEGER DEFAULT 0, /* used to find deleted files, the scanner sets this to 1 when found on fs */
        deleted INTEGER DEFAULT 0,
        to_checksum INTEGER DEFAULT 0,
        sha256 TEXT DEFAULT '',
        last_changed_rev INTEGER DEFAULT 0,
        updated INTEGER DEFAULT 0
        )
    )#").exec();

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS files_vclock (
        path TEXT NOT NULL,
        key TEXT NOT NULL,
        value INTEGER DEFAULT 0,
        FOREIGN KEY(path) REFERENCES files(path)
        )
    )#").exec();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_files_vlock_path ON files_vclock(path))#").exec();


    //
    // PEER FILES
    //
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS peer_files (
        path TEXT NOT NULL,
        tmp_path TEXT DEFAULT '',
        peer TEXT NOT NULL,
        mtime TEXT,
        size INTEGER,
        mode INTEGER,
        sha256 TEXT,
        deleted INTEGER DEFAULT 0
        )
    )#").exec();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_peer_files_path ON peer_files(path))#").exec();

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS peer_files_vclock (
        path TEXT NOT NULL,
        key TEXT NOT NULL,
        value INTEGER DEFAULT 0,
        FOREIGN KEY(path) REFERENCES peer_files(path)
        )
    )#").exec();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_peer_files_vlock_path ON peer_files_vclock(path))#").exec();


    //
    // SHARE
    //
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS share (
        share_id TEXT PRIMARY KEY,
        peer_id TEXT NOT NULL,
        psk_rw TEXT NOT NULL,
        psk_ro TEXT NOT NULL,
        psk_untrusted TEXT NOT NULL,
        pkc_rw TEXT,
        pkc_ro TEXT
        )
    )#").exec();
    // manifest vclock, ours and the ones from peers

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS share_revision (
        peer_id TEXT NOT NULL,
        revision INTEGER DEFAULT 0,
        FOREIGN KEY(peer_id) REFERENCES share(peer_id)
        )
    )#").exec();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_share_revision_peer_id ON share_revision(peer_id))#").exec();


}


void Share::scan()
{
    m_scan_in_progress = true;
    m_scan_it = make_unique<bfs::recursive_directory_iterator>(m_path);
}

void Share::step()
{
    const bool scan_finished = scan_step();
    const bool cksum_finished = cksum_step();
    const bool m_scan_in_progress = ! scan_finished || ! cksum_finished;
    return scan_finished && cksum_finished;
}


/**
 * scan m_scan_batch_sz files @returns true if finished, false if more to do
 */
bool Share::scan_step()
{
    if (! m_scan_it)
        return true;

    size_t batch_i = 0; // number of files in this batch so far
    bfs::recursive_directory_iterator& it = *m_scan_it;
    bfs::recursive_directory_iterator end;
    for ( ; it != end && batch_i < m_scan_batch_sz; ++it, ++batch_i)
    {
        const auto& dentry = *it;
        if (dentry.status().type() == bfs::regular_file)
        {
            File f;
            // we get the path relative to the share
            bfs::path fpath = get_tail(dentry.path(), it.level() + 1);
            assert(fpath.is_relative());
            f.path = fpath.string();
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
    if (it == end)
    {
        m_scan_it.reset();
        on_scan_finished();
        return true;
    }
    else
        return false;
}

void Share::on_scan_finished()
{
    // TODO mark not found files as deleted
}

void Share::cksum_step()
{
}


std::unique_ptr<MFile> Share::get_file_info(const std::string& path)
{
    unique_ptr<MFile> result;
    sqlite3pp::query file_q(m_db, "SELECT path, mtime, size, mode, scan_found, deleted, to_checksum, sha256, last_changed_rev, updated FROM files WHERE path = :path");
    file_q.bind(":path", path);

    bool found = false;
    for (const auto& row: file_q)
    {
        assert(! found); // path must be unique, it's pk
        result = make_unique<MFile>();
        assert(row.get<std::string>(0) == path);
        mfile_from_row(*result, row);
        found = true;
    }
    return move(result);
}

void Share::set_file_info(const MFile& f)
{
    sqlite3pp::command file_i(m_db, "INSERT INTO files (path, mtime, size, mode, sha256, deleted, last_changed_rev, scan_found, updated) VALUES (?,?,?,?,?,?,?,?,?)");
    file_i.bind(1, f.path);
    file_i.bind(2, f.mtime);
    file_i.bind(3, static_cast<long long int>(f.size));
    file_i.bind(4, static_cast<int>(f.mode));
    file_i.bind(5, f.sha256);
    file_i.bind(6, static_cast<int>(f.deleted));

    file_i.exec();
}

void Share::checksum_thread()
{

    sqlite3pp::query file_q(m_db, "SELECT path, mtime, size, mode, sha256, deleted FROM files WHERE sha256 = '' ORDER BY path");
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

void Share::scan_found(ScanFile&& scan_file)
{
    // TODO: add bytes to checksum for stats
    unique_ptr<MFile> mfile = get_file_info(scan_file.path);
    if (mfile)
    {
        if (scan_file.mtime != mfile->mtime
            || scan_file.size != mfile->size)
        {
            *mfile = scan_file;
            mfile->to_checksum = true;
            set_file_info(*mfile);
        }
        else
        {
            *mfile = scan_file;
            mfile->to_checksum = false;
            set_file_info(*mfile);
        }
    }
    else
    {
        MFile mfile_;
        mfile_ = scan_file;
        mfile.to_checksum = true;
        set_file_info(mfile_);
    }
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
