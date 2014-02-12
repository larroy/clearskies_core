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
using namespace std;

namespace
{




} // end anon ns

namespace cs
{
namespace share
{


void MFile::from_row(const sqlite3pp::query::rows& row)
{
    path = row.get<string>(0);
    mtime = row.get<string>(1);
    size = row.get<u64>(2);
    mode = row.get<int>(3);
    scan_found = row.get<bool>(4);
    deleted = row.get<bool>(5);
    to_checksum = row.get<bool>(6);
    sha256 = row.get<string>(7);
    last_changed_rev = row.get<u64>(8);
    updated = row.get<bool>(9);
}

void MFile::gone(u64* share_rev)
{
    assert(share_rev);
    // file disappeared
    size = 0;
    mode = 0;
    scan_found = true;
    deleted = true;
    to_checksum = false;
    sha256.clear();
    last_changed_rev = ++*share_rev;
}

Share::Share_iterator::Share_iterator():
    m_query()
    , m_query_it()
    , m_file()
    , m_file_set()
{}

Share::Share_iterator::Share_iterator(Share& share):
    m_query(make_unique<sqlite3pp::query>(share.m_db, "SELECT * FROM files ORDER BY path"))
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
        m_file.from_row(*m_query_it);
        m_file_set = true;
    }
    return m_file;
}


Share::Checksummer::Checksummer(Share& share):
     r_share(share)
    , m_to_cksum_it()
    , m_c256()
    , m_file()
    , m_is()

{
    sha2::SHA256_Init(&m_c256);
}



bool Share::Checksummer::step()
{
    for(size_t nblock = 0; nblock < r_share.m_cksum_batch_sz;)
    {
        if (m_is)
        {
            do_block();
            ++nblock;
        }
        else
        {
            bool more = next_file();
            if (! more)
                return false;
        }
    }
    return true;
}

void Share::Checksummer::do_block()
{
    assert(m_is);
    std::array<char, Share::s_cksum_block_sz> rbuff;
    m_is->read(rbuff.data(), rbuff.size());
    sha2::SHA256_Update(&m_c256, (const cs::u8*) rbuff.data(), m_is->gcount());
    if (! *m_is)
    {
        // EOF 
        string sha256(SHA256_DIGEST_STRING_LENGTH, 0);
        sha2::SHA256_End(&m_c256, &sha256[0]);
        sha256.resize(sha256.size() - 1);
        // FIXME check if file vanished!!
        r_share.m_update_hash_q.reset().bind(":sha256", sha256).bind(":path", m_file.path);
        r_share.m_update_hash_q.execute();
    }
}


bool Share::Checksummer::next_file()
{
    r_share.m_to_cksum_q.reset();
    m_to_cksum_it = r_share.m_to_cksum_q.begin();
    if (m_to_cksum_it == r_share.m_to_cksum_q.end())
        return false;
    m_file.from_row(*m_to_cksum_it);
    bfs::path fullpath = r_share.m_path / bfs::path(m_file.path);
    m_is = make_unique<bfs::ifstream>(fullpath, ios_base::in | ios_base::binary);
    if (! *m_is)
    {
        m_file.gone(&r_share.m_revision);
        r_share.set_file_info(m_file);
        m_is.reset();
    }
    m_is->exceptions(ios::badbit);
    sha2::SHA256_Init(&m_c256);
    return true;
}

Share::Share(const std::string& share_path, const std::string& dbpath):
      m_path(share_path)
    , m_revision()
    , m_db(dbpath.c_str())
    , m_db_path(dbpath)
    , m_scan_in_progress()
    , m_scan_batch_sz(256)
    , m_scan_it()
    , m_scan_found_count()
    , m_scan_duration_s()
    , m_set_file_info_q(m_db)
    , m_cksum_batch_sz(8)
    , m_to_cksum_q(m_db)
    , m_cksummer(*this)
    , m_update_hash_q(m_db)
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
    initialize_statements();
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
        cmd.execute();

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
    )#").execute();

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS files_vclock (
        path TEXT NOT NULL,
        key TEXT NOT NULL,
        value INTEGER DEFAULT 0,
        FOREIGN KEY(path) REFERENCES files(path)
        )
    )#").execute();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_files_vlock_path ON files_vclock(path))#").execute();


    //
    // PEER FILES
    //
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS peer_files (
        path TEXT NOT NULL,
        peer TEXT NOT NULL,
        tmp_path TEXT DEFAULT '',
        mtime TEXT,
        size INTEGER,
        mode INTEGER,
        sha256 TEXT,
        deleted INTEGER DEFAULT 0
        )
    )#").execute();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_peer_files_path ON peer_files(path))#").execute();

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS peer_files_vclock (
        path TEXT NOT NULL,
        peer TEXT NOT NULL,
        key TEXT NOT NULL,
        value INTEGER DEFAULT 0,
        FOREIGN KEY(path) REFERENCES peer_files(path)
        )
    )#").execute();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_peer_files_vlock_path ON peer_files_vclock(path))#").execute();


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
    )#").execute();
    // manifest vclock, ours and the ones from peers

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS share_revision (
        peer_id TEXT NOT NULL,
        revision INTEGER DEFAULT 0,
        FOREIGN KEY(peer_id) REFERENCES share(peer_id)
        )
    )#").execute();
    sqlite3pp::command(m_db, R"#(CREATE INDEX IF NOT EXISTS i_share_revision_peer_id ON share_revision(peer_id))#").execute();


}


void Share::initialize_statements()
{
    m_set_file_info_q.prepare("INSERT INTO files (path, mtime, size, mode, scan_found, deleted, to_checksum, sha256, last_changed_rev, updated) VALUES (?,?,?,?,?,?,?,?,?,?)");
    m_to_cksum_q.prepare("SELECT * FROM files WHERE to_checksum != 0 ORDER BY path");
    m_update_hash_q.prepare("UPDATE files SET sha256 = :sha256, to_checksum = 0 WHERE path = :path");
}

std::unique_ptr<MFile> Share::get_file_info(const std::string& path)
{
    unique_ptr<MFile> result;
    sqlite3pp::query file_q(m_db, "SELECT * FROM files WHERE path = :path");
    file_q.bind(":path", path);

    bool found = false;
    for (const auto& row: file_q)
    {
        assert(! found); // path must be unique, it's pk
        result = make_unique<MFile>();
        assert(row.get<std::string>(0) == path);
        result->from_row(row);
        found = true;
    }
    return move(result);
}

void Share::set_file_info(const MFile& f)
{
    m_set_file_info_q.reset();
    m_set_file_info_q.bind(1, f.path);
    m_set_file_info_q.bind(2, f.mtime);
    m_set_file_info_q.bind(3, f.size);
    m_set_file_info_q.bind(4, f.mode);
    m_set_file_info_q.bind(5, f.mode);
    m_set_file_info_q.bind(5, f.sha256);
    m_set_file_info_q.bind(6, f.deleted);
    m_set_file_info_q.execute();
}




/**
 * Initialize the directory iterator, so scan_step does work
 */
void Share::scan()
{
    m_scan_in_progress = true;
    m_scan_it = make_unique<bfs::recursive_directory_iterator>(m_path);
    m_scan_found_count = 0;
    time(&m_scan_duration_s);
}

bool Share::scan_step()
{
    if (m_scan_in_progress == false)
    {
        assert(false);
        return false;
    }
    const bool scan_more = fs_scan_step();
    const bool cksum_more = m_cksummer.step();
    const bool m_scan_in_progress = scan_more || cksum_more;
    if (! m_scan_in_progress)
        on_scan_finished();
    return m_scan_in_progress;
}


/**
 * scan m_scan_batch_sz files @returns true if finished, false if more to do
 */
bool Share::fs_scan_step()
{
    if (! m_scan_it)
        return false;

    size_t batch_i = 0; // number of files in this batch so far
    bfs::recursive_directory_iterator& it = *m_scan_it;
    bfs::recursive_directory_iterator end;
    for ( ; it != end && batch_i < m_scan_batch_sz; ++it, ++batch_i)
    {
        const auto& dentry = *it;
        if (dentry.status().type() == bfs::regular_file)
        {
            ScanFile f;
            // we get the path relative to the share
            bfs::path fpath = get_tail(dentry.path(), it.level() + 1);
            assert(fpath.is_relative());
            f.path = fpath.string();
            f.mtime = utils::isotime(bfs::last_write_time(dentry.path()));
            f.size = bfs::file_size(dentry.path());
            f.mode = dentry.status().permissions();
            scan_found(f);

#if 0
            cout << f.path << endl;
            cout << f.mtime << endl;
            cout << oct << f.mode << dec << endl;
            cout << f.size << endl;
            cout << endl;
#endif
        }

    }
    // scan finished
    if (it == end)
    {
        m_scan_it.reset();
        return false;
    }
    else
        return true;
}

void Share::on_scan_finished()
{
    time_t scan_end = 0;
    time(&scan_end);
    m_scan_duration_s = scan_end - m_scan_duration_s;
    // TODO mark not found files as deleted
}


void Share::scan_found(const ScanFile& scan_file)
{
    // TODO: add bytes to checksum for stats
    unique_ptr<MFile> mfile = get_file_info(scan_file.path);
    ++m_scan_found_count;
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
        mfile_.to_checksum = true;
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
