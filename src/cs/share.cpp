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
#include "boost/format.hpp"
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
    last_changed_rev = row.get<string>(8);
    last_changed_by = row.get<string>(9);
    updated = row.get<bool>(10);
}

void MFile::was_deleted(const std::string& peer_id, const std::string& revision)
{
    // file disappeared
    size = 0;
    mode = 0;
    scan_found = true;
    deleted = true;
    to_checksum = false;
    sha256.clear();
    last_changed_rev = revision;
    last_changed_by = peer_id;
    updated = true;
}


FrozenManifestIterator::FrozenManifestIterator(FrozenManifest& frozen_manifest, bool is_end):
    r_frozen_manifest(frozen_manifest)
    , m_query()
    , m_query_it()
    , m_file()
    , m_file_set()
    , m_is_end(is_end)
{
    assert(is_end);
}

FrozenManifestIterator::FrozenManifestIterator(FrozenManifest& frozen_manifest):
    r_frozen_manifest(frozen_manifest)
    , m_query_str(boost::str(boost::format(R"#(
        SELECT
            path
            , mtime
            , size
            , mode
            , scan_found
            , deleted
            , to_checksum
            , sha256
            , last_changed_rev
            , last_changed_by
            , updated
        FROM %1% ORDER BY path
    )#") % r_frozen_manifest.m_table))
    , m_query(make_unique<sqlite3pp::query>(r_frozen_manifest.r_share.m_db, m_query_str.c_str()))
    , m_query_it(m_query->begin())
    , m_file()
    , m_file_set()
    , m_is_end()
{
}


void FrozenManifestIterator::increment()
{
    ++m_query_it;
    m_file_set = false;
}

MFile& FrozenManifestIterator::dereference() const
{
    assert(! m_is_end);
    if (! m_file_set)
    {
        m_file.from_row(*m_query_it);
        m_file_set = true;
    }
    return m_file;
}

FrozenManifest::FrozenManifest(const std::string& peer_id, Share& share, const std::map<std::string, std::string>& since):
    m_peer_id(peer_id)
    , r_share(share)
    , m_table("frozen_files_" + peer_id)
    , m_since(since)
{
    {
        // The temporary table should not exist for this peer already
        sqlite3pp::query q_cnt_tbl(r_share.m_db, R"#(SELECT COUNT(*) FROM sqlite_master WHERE type='table' and name=?)#");
        q_cnt_tbl.bind(1, m_table);
        const bool exists = q_cnt_tbl.fetchone().get<u64>(0) != 0;
        assert(! exists);
    }
    {
        /*
         *
         * SELECT * FROM files
         * WHERE last_changed_by = 'A' AND last_changed_rev > 2
         * OR last_changed_by = 'B' AND last_changed_rev > 1
         * OR last_changed_by NOT IN ('A', 'B')
         *
         */
        ostringstream where;
        if (! since.empty())
        {
            where << "AND (";
            {
                for (const auto& x: since)
                {
                    if (&x != &*since.begin())
                        where << "OR ";
                    where << "last_changed_by = '" << x.first << "' AND last_changed_rev > '" << x.second << "'\n";
                }
            }

            where << "OR last_changed_by NOT IN (";
            for (const auto& x: since)
            {
                if (&x != &*since.begin())
                    where << ", ";
                where << "'" << x.first << "'";
            }
            where << ")\n";
        }
        // Freeze the manifest into a temporary table
        const string query = boost::str(boost::format(R"#(CREATE TEMPORARY TABLE %1% AS
            SELECT * FROM files
            WHERE
                scan_found = 0
                AND deleted = 0
                AND to_checksum = 0
                AND sha256 != ''
                %2%
        )#") % m_table % where.str());
        sqlite3pp::command(r_share.m_db, query.c_str()).execute();
    }
#if 0
    {
        // Get the update vector, given the latest changes to files
        sqlite3pp::query q(r_share.m_db, R"#(SELECT last_changed_by, max(last_changed_rev) FROM t GROUP BY last_changed_by)#");
        for (const auto& row: q)
            m_since[row.get<string>(0)] = row.get<string>(1);
    }
#endif
}

FrozenManifest::~FrozenManifest()
{
    const string q = boost::str(boost::format("DROP TABLE %1%") % m_table);
    sqlite3pp::command(r_share.m_db, q.c_str()).execute();
}


Share::Share_iterator::Share_iterator():
    m_query()
    , m_query_it()
    , m_file()
    , m_file_set()
{}

Share::Share_iterator::Share_iterator(Share& share):
    m_query(make_unique<sqlite3pp::query>(share.m_db, R"#(
        SELECT
            path
            , mtime
            , size
            , mode
            , scan_found
            , deleted
            , to_checksum
            , sha256
            , last_changed_rev
            , last_changed_by
            , updated
        FROM files ORDER BY path
    )#"))
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
        if (! bfs::exists(r_share.fullpath(m_file.path)))
        {
            // check if file vanished one last time
            m_file.was_deleted(r_share.m_peer_id, r_share.m_revision.get_str());
            ++r_share.m_revision;
        }
        else
        {
            m_file.sha256 = move(sha256);
            m_file.to_checksum = false;
            m_file.updated = true;
        }
        r_share.update_mfile(m_file);
        m_is.reset();
    }
}


bool Share::Checksummer::next_file()
{
    // the query needs to be rerun, since checksumming runs interwinded with file scanning, so there
    // could be new files to checksum added on every step. Another solution is to first scan then
    // checksum, but this way we should be utilizing the CPU more.
    r_share.m_select_to_cksum_q.reset();
    const auto to_cksum_it = r_share.m_select_to_cksum_q.begin();

    // There are no more files to checksum
    if (to_cksum_it == r_share.m_select_to_cksum_q.end())
        return false;

    m_file.from_row(*to_cksum_it);
    m_is = make_unique<bfs::ifstream>(r_share.fullpath(bfs::path(m_file.path)), ios_base::in | ios_base::binary);

    if (! *m_is) // note that we are not checking the pointer, but the stream
    {
        // file can't be opened, it has been deleted
        m_file.was_deleted(r_share.m_peer_id, r_share.m_revision.get_str());
        ++r_share.m_revision;
        r_share.update_mfile(m_file);
        m_is.reset();
    }
    m_is->exceptions(ios::badbit);
    sha2::SHA256_Init(&m_c256);
    return true;
}

Share::Share(const std::string& share_path, const std::string& dbpath):
      m_path(share_path)
    , m_revision(0)
    , m_db(dbpath.c_str())
    , m_db_path(dbpath)
    , m_insert_mfile_q(m_db)
    , m_update_mfile_q(m_db)
    , m_scan_in_progress()
    , m_scan_batch_sz(256)
    , m_scan_it()
    , m_scan_found_count()
    , m_scan_duration_s()
    , m_select_not_scan_found_q(m_db)
    , m_update_scan_found_false_q(m_db)
    , m_cksum_batch_sz(8)
    , m_select_to_cksum_q(m_db)
    , m_cksummer(*this)
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
    performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA page_size = 1024"));
    performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA cache_size = -16384"));
    performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA fullfsync = 0"));
    // performance_adjusts.emplace_back(sqlite3pp::command(m_db, "PRAGMA journal_mode = OFF"));

    for (sqlite3pp::command& cmd: performance_adjusts)
        cmd.execute();


    //
    // SHARE IDENTITY, KEYS
    //
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS share (
        share_id TEXT PRIMARY KEY,
        revision TEXT DEFAULT '0',
        peer_id TEXT NOT NULL,
        psk_rw TEXT NOT NULL,
        psk_ro TEXT NOT NULL,
        psk_untrusted TEXT NOT NULL,
        pkc_rw TEXT,
        pkc_ro TEXT
        )
    )#").execute();

    init_or_read_share_identity();

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
        last_changed_rev TEXT DEFAULT '0', /* revision in which this file was changed */
        last_changed_by TEXT DEFAULT '', /* peer that changed this file last */
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


}


void Share::initialize_statements()
{
    m_insert_mfile_q.prepare("INSERT INTO files (path, mtime, size, mode, scan_found, deleted, to_checksum, sha256, last_changed_rev, last_changed_by, updated) VALUES (?,?,?,?,?,?,?,?,?,?,?)");
    m_update_mfile_q.prepare(R"#(UPDATE files SET
        mtime = ?,
        size = ?,
        mode = ?,
        scan_found = ?,
        deleted = ?,
        to_checksum = ?,
        sha256 = ?,
        last_changed_rev = ?,
        last_changed_by = ?,
        updated = ?
    WHERE path = ?
    )#");

    m_select_not_scan_found_q.prepare(R"#(SELECT
        path,
        mtime,
        size,
        mode,
        scan_found,
        deleted,
        to_checksum,
        sha256,
        last_changed_rev,
        last_changed_by,
        updated
    FROM
        files
    WHERE scan_found = 0 ORDER BY path)#");
    m_update_scan_found_false_q.prepare("UPDATE files SET scan_found = 0");
    m_select_to_cksum_q.prepare("SELECT * FROM files WHERE to_checksum != 0 ORDER BY path");
}


void Share::init_or_read_share_identity()
{
    assert(m_share_id.empty());
    assert(m_peer_id.empty());

    sqlite3pp::query q(m_db, R"#(SELECT
        share_id,
        revision,
        peer_id,
        psk_rw,
        psk_ro,
        psk_untrusted,
        pkc_rw,
        pkc_ro
    FROM share)#");
    bool found = false;
    for (const auto& row: q)
    {
        assert(! found); // path must be unique, it's pk

        m_share_id = row.get<string>(0);
        m_revision = row.get<string>(1);
        m_peer_id = row.get<string>(2);
        m_psk_rw = row.get<string>(3);
        m_psk_ro = row.get<string>(4);
        m_psk_untrusted = row.get<string>(5);
        m_pkc_rw = row.get<string>(6);
        m_pkc_ro = row.get<string>(7);

        found = true;
    }

    if (! found)
    {
        m_share_id = utils::bin_to_hex(utils::random_bytes(32));
        m_peer_id = utils::bin_to_hex(utils::random_bytes(16));
        m_psk_rw = utils::bin_to_hex(utils::random_bytes(16));
        m_psk_ro = utils::bin_to_hex(utils::random_bytes(16));
        m_psk_untrusted = utils::random_bytes(16);
        // FIXME PKC
        //
        sqlite3pp::command q(m_db, "INSERT INTO share (share_id, revision, peer_id, psk_rw, psk_ro, psk_untrusted, pkc_rw, pkc_ro) VALUES (?,?,?,?,?,?,?,?)");
        q.bind(1, m_share_id);
        q.bind(2, m_revision.get_str());
        q.bind(3, m_peer_id);
        q.bind(4, m_psk_rw);
        q.bind(5, m_psk_ro);
        q.bind(6, m_psk_untrusted);
        q.bind(7, "");
        q.bind(8, "");
        q.execute();
    }
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
        UNUSED(found);
        result = make_unique<MFile>();
        assert(row.get<std::string>(0) == path);
        result->from_row(row);
        found = true;
    }
    return move(result);
}

void Share::insert_mfile(const MFile& f)
{
    m_insert_mfile_q.reset();
    m_insert_mfile_q.bind(1, f.path);
    m_insert_mfile_q.bind(2, f.mtime);
    m_insert_mfile_q.bind(3, f.size);
    m_insert_mfile_q.bind(4, f.mode);
    m_insert_mfile_q.bind(5, f.scan_found);
    m_insert_mfile_q.bind(6, f.deleted);
    m_insert_mfile_q.bind(7, f.to_checksum);
    m_insert_mfile_q.bind(8, f.sha256);
    m_insert_mfile_q.bind(9, f.last_changed_rev);
    m_insert_mfile_q.bind(10, f.last_changed_by);
    m_insert_mfile_q.bind(11, f.updated);
    m_insert_mfile_q.execute();
}

void Share::update_mfile(const MFile& f)
{
    m_update_mfile_q.reset();
    assert(! f.path.empty());
    m_update_mfile_q.bind(1, f.mtime);
    m_update_mfile_q.bind(2, f.size);
    m_update_mfile_q.bind(3, f.mode);
    m_update_mfile_q.bind(4, f.scan_found);
    m_update_mfile_q.bind(5, f.deleted);
    m_update_mfile_q.bind(6, f.to_checksum);
    m_update_mfile_q.bind(7, f.sha256);
    m_update_mfile_q.bind(8, f.last_changed_rev);
    m_update_mfile_q.bind(9, f.last_changed_by);
    m_update_mfile_q.bind(10, f.updated);
    m_update_mfile_q.bind(11, f.path);
    m_update_mfile_q.execute();
    assert(m_db.changes() == 1);
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
            MFile f;
            // we get the path relative to the share
            bfs::path fpath = get_tail(dentry.path(), it.level() + 1);
            assert(fpath.is_relative());
            f.path = fpath.string();
            f.mtime = utils::isotime(bfs::last_write_time(dentry.path()));
            f.size = bfs::file_size(dentry.path());
            f.mode = dentry.status().permissions();
            f.scan_found = true;
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
    // save the time scan took
    time_t scan_end = 0;
    time(&scan_end);
    m_scan_duration_s = scan_end - m_scan_duration_s;

    // the files which coulnd't be found are marked as deleted
    m_select_not_scan_found_q.reset();
    for (const auto& row: m_select_not_scan_found_q)
    {
        MFile file;
        file.from_row(row);
        file.was_deleted(m_peer_id, m_revision.get_str());
        ++m_revision;
        update_mfile(file);
    }

    // reset scan_found for all the files
    m_update_scan_found_false_q.reset();
    m_update_scan_found_false_q.execute();
}


void Share::scan_found(MFile& scan_file)
{
    // TODO: add bytes to checksum for stats
    assert(scan_file.scan_found);
    unique_ptr<MFile> mfile = get_file_info(scan_file.path);
    ++m_scan_found_count;
    if (mfile) // found
    {
        *mfile = scan_file;
        bool changed = false;
        if (scan_file.mtime != mfile->mtime || scan_file.size != mfile->size);
        {
            mfile->to_checksum = true;
            changed = true;
        }
        if (changed || scan_file.mode != mfile->mode)
        {
            mfile->last_changed_rev = m_revision.get_str();
            ++m_revision;
            mfile->last_changed_by = m_peer_id;
            changed = true;
        }
        if (changed)
            update_mfile(*mfile);
    }
    else
    {
        scan_file.to_checksum = true;
        insert_mfile(scan_file);
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
