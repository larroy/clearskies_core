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
#include "int_types.h"
#include "file.hpp"
#include "boost_fs_fwd.hpp"
#include "sqlite3pp/sqlite3pp.h"
#include <boost/iterator/iterator_facade.hpp>
#include <array>
#include <string>
#include <thread>
namespace sha2
{
#include "sha2/sha2.h"
}



namespace cs
{
namespace share
{

struct ScanFile 
{
    ScanFile():
        path()
        , mtime()
        , size()
        , mode()
        , scan_found(true)
    {}
    std::string path;
    std::string mtime;
    u64 size;
    u16 mode;
    bool scan_found;
};

struct MFile: ScanFile
{
    MFile():
        deleted()
        , to_checksum()
        , sha256()
        , last_changed_rev()
        , updated()
    {}

    MFile& operator=(const ScanFile& x)
    {
        if (&x == this)
            return *this;

        std::tie(path, mtime, size, mode, scan_found) =
            std::tie(x.path, x.mtime, x.size, x.mode, x.scan_found);
        return *this;
    }

    void from_row(const sqlite3pp::query::rows& row);

    void gone(u64* share_rev);

    bool deleted;
    bool to_checksum;
    std::string sha256;
    u64 last_changed_rev;
    bool updated;
};

/**
 * Filesystem scan is done in two passes, first files are monitored for size and time changes, then
 * if this indicates any change or the file is new they ar marked to be checksummed
 *
 * Procedure to commit a file to a share:
 *  - An updated file from another client is downloaded into a temporary directory outside the share
 *  together with its vector clock.
 *  - Once the file is fully downloaded, it's checksum is calculated, if it matches the file is
 *  commited to the share (so a scan is not in place at the same time). 
 *  - On commit if the vclock of the new file is descendant of the file we already have, it's
 *  replaced, otherwise this file is marked as conflicted and respective copies are saved in the
 *  share.
 *  - The scanner should account for conflicted files not to be treated as "new files".
 *
 * Scan steps:
 * 1: mark scan_found = 0
 * 1: scan
 *  1.1: cksum & rescan
 *  1.2: send updates
 * mark remaining scan_found = 0 files as deleted
 *
 */
class Share
{
public:

    /**
     * Iterator to allow for(const auto& file: share) idiom
     */
    class Share_iterator: public boost::iterator_facade<Share_iterator, MFile, boost::single_pass_traversal_tag>
    {
    public:
        Share_iterator();
        explicit Share_iterator(Share&);

    private:
        friend class boost::iterator_core_access;
        void increment();
        bool equal(const Share_iterator& other) const
        {
            return m_query_it == other.m_query_it;
        }

        MFile& dereference() const;

        std::unique_ptr<sqlite3pp::query> m_query;
        sqlite3pp::query::query_iterator m_query_it;
        mutable MFile m_file;
        mutable bool m_file_set;
    };

    class Checksummer
    {
    public:
        Checksummer(Share&);
        bool step();

    private:
        void do_block();
        /// @returns true if there's more
        bool next_file();
        Share& r_share;
        sqlite3pp::query::query_iterator m_to_cksum_it;
        sha2::SHA256_CTX  m_c256;
        MFile m_file;
        std::unique_ptr<bfs::ifstream> m_is;
    };


    Share(const std::string& share_path, const std::string& dbpath = ":memory:");
    void initialize_tables();
    void initialize_statements();

    /**
     * get an iterator to traverse all the files in the share, changes to the file _don't_ update the
     * database, so the iterator behaves as const
     */
    Share_iterator begin()
    {
        return Share_iterator(*this);
    }

    Share_iterator end()
    {
        return Share_iterator();
    }

    /// @returns file metadata given a path, null if there's no such file
    std::unique_ptr<MFile> get_file_info(const std::string& path);

    /// save File metadata on the Share
    void set_file_info(const MFile&);

    void scan();

    /// @returns true if there's more to do, false otherwise, meaning scan and cksum finished
    bool scan_step();


private:
    /// @returns true if there's more to do, this does one step in the scan part
    bool fs_scan_step();

    // called once after scanning and checksumming finishes
    void on_scan_finished();

    /// @returns true if a scan is in progress
    bool scan_in_progress() const { return m_scan_in_progress; }

#if 0
    size_t scan_total() const { assert(0); }
    size_t scan_done() const { assert(0); }
    bool checksum_in_progress() const { assert(0); }
    size_t checksum_total() const { assert(0); }
    size_t checksum_done() const { assert(0); }
#endif

    /// actions to perform for each scanned file
    void scan_found(const ScanFile& file);

public:
    std::string m_path;
    u64 m_revision;
    sqlite3pp::database m_db;
    std::string m_db_path;

    bool m_scan_in_progress;
    /// number of files to scan at once
    size_t m_scan_batch_sz;
    std::unique_ptr<bfs::recursive_directory_iterator> m_scan_it;
    size_t m_scan_found_count;
    std::time_t m_scan_duration_s;

    sqlite3pp::command m_set_file_info_q;

    /// cksum buffer size
    static const size_t s_cksum_block_sz = 65536;
    /// number of buffers to cksum at once, total s_cksum_block_sz * m_cksum_batch_sz bytes will be
    /// read from disk before yielding
    size_t m_cksum_batch_sz;
    sqlite3pp::query m_to_cksum_q;
    Checksummer m_cksummer;
    sqlite3pp::command m_update_hash_q;

    /// share id, shared publicly
    std::array<u8, 32> m_share_id;
    std::array<u8, 16> m_peer_id;

    /// pre-shared key read-write
    std::array<u8, 16> m_psk_rw;
    /// pre-shared key read-only
    std::array<u8, 16> m_psk_ro;
    /// pre-shared key untrusted
    std::array<u8, 16> m_psk_untrusted;

    /// private keys
    std::array<u8, 256> m_pkc_rw;
    std::array<u8, 256> m_pkc_ro;
};

/// returns a path with the last tail number of components
bfs::path get_tail(const bfs::path& path, size_t tail);


} // end ns
} // end ns
