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
#include "file.hpp"
#include "boost_fs_fwd.hpp"
#include "sqlite3pp/sqlite3pp.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <array>
#include <map>
#include <string>
#include <thread>
#include <gmpxx.h>
namespace sha2
{
#include "sha2/sha2.h"
}


/*
 *
 * /!\ Keep the same order of functions in the .hpp and .cpp file
 * /!\ Keep the same order of member variables in the db table and C++ data structure
 *
 */


namespace cs
{
namespace share
{

class Share;


struct MFile
{
    MFile():
        path()
        , mtime()
        , size()
        , mode()
        , scan_found()
        , deleted()
        , to_checksum()
        , sha256()
        , last_changed_rev()
        , last_changed_by()
        , updated()
    {}

    bool operator==(const MFile& o) const
    {
        return std::tie(path, mtime, size, mode, scan_found, deleted, to_checksum, sha256, last_changed_rev, last_changed_by, updated) ==
            std::tie(o.path, o.mtime, o.size, o.mode, o.scan_found, o.deleted, o.to_checksum, o.sha256, o.last_changed_rev, o.last_changed_by, o.updated);

    }

    void from_row(const sqlite3pp::query::rows& row);

    /// mark file as deleted, @param share_rev is incremented @pre share_rev is != 0
    void was_deleted(const std::string& peer_id, const std::string& rev);

    std::string path;
    std::string mtime;
    u64 size;
    u16 mode;
    bool scan_found;
    bool deleted;
    bool to_checksum;
    std::string sha256;
    std::string last_changed_rev;
    std::string last_changed_by;
    bool updated;
};

class FrozenManifest;

/**
 * An iterator over a frozen manifest, @sa FrozenManifest
 *
 * Copies the manifest into a temporary table so the peer has a stable view over it.
 */
class FrozenManifestIterator: public boost::iterator_facade<FrozenManifestIterator, MFile, boost::single_pass_traversal_tag>
{
friend class boost::iterator_core_access;
public:
    FrozenManifestIterator(FrozenManifest&);
    FrozenManifestIterator(FrozenManifest&, bool is_end);
    FrozenManifestIterator(FrozenManifestIterator&&) = default;
    FrozenManifestIterator& operator=(FrozenManifestIterator&&) = default;

private:
    void increment();
    bool equal(const FrozenManifestIterator& other) const
    {
        return m_query_it == other.m_query_it;
    }

    MFile& dereference() const;

    FrozenManifest& r_frozen_manifest;
    const std::string m_query_str;
    std::unique_ptr<sqlite3pp::query> m_query;
    sqlite3pp::query::query_iterator m_query_it;
    mutable MFile m_file;
    mutable bool m_file_set;
    bool m_is_end;
};

/**
 * A frozen view over the manifest for a peer, this is a proxy object to create iterators
 *
 * It copies the manifest in a temporary table that remains stable during the process of
 * transmitting the manifest
 */
class FrozenManifest
{
public:
    /**
     * @param[in] peer_id the _other_ peer id which is requesting this manifest
     * @param[in] share the share
     */
    FrozenManifest(const std::string& peer_id, Share& share, const std::map<std::string, std::string>& since);

    ~FrozenManifest();

    FrozenManifestIterator begin()
    {
        return FrozenManifestIterator(*this);
    }

    FrozenManifestIterator end()
    {
        return FrozenManifestIterator(*this, true);
    }

    std::string m_peer_id;
    Share& r_share;
    std::string m_table;
    std::map<std::string, std::string> m_since;
};

/**
 * Filesystem scan is done in two passes, first files are checked for size and time changes, then
 * if this indicates any change or the file is new they ar marked to be checksummed (to_checksum).
 *
 * Filesystem scan and cheksum are done in steps in order not to starve the event loop.
 *
 * Once a scan is started through Share::scan(), Share::scan_step should be called until it returns
 * false.
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
     * Iterate through all the files in the share database.
     * Changes to files through the iterator don't change the database or produce any side-effects. @sa Share::m_query
     * @code
     * for(const auto& file: share)
     * {
     *      // do something with file
     * }
     * @endcode
     */
    class Share_iterator: public boost::iterator_facade<Share_iterator, MFile, boost::single_pass_traversal_tag>
    {
    friend class boost::iterator_core_access;
    public:
        Share_iterator();
        explicit Share_iterator(Share&);

    private:
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

    /**
     * Incremental checksum of files.
     * Files marked with to_checksum != 0 will be processed in batches controlled by
     * Share::m_cksum_batch_sz and Share::s_cksum_block_sz
     */
    class Checksummer
    {
    public:
        Checksummer(Share&);
        /**
         * step should be called until it returns false, then all the files to_checksum are processed.
         * It checksums Share::m_cksum_batch_sz blocks or less. @returns false if there are no more
         * blocks or files to checksum, true otherwise.
         */
        bool step();

    private:
        /**
         * reads a single block from a file and updates the m_c256 context, on EOF we update the db
         * and reset m_is
         */
        void do_block();
        /// @returns true if there's more
        bool next_file();
        Share& r_share;
        sha2::SHA256_CTX  m_c256;
        MFile m_file;
        /// when it's set means we are in the middle of checksumming a file
        std::unique_ptr<bfs::ifstream> m_is;
    };


    Share(const std::string& share_path, const std::string& dbpath = ":memory:");
    Share(Share&&) = default;
    Share& operator=(Share&&) = default;

    void initialize_tables();
    void initialize_statements();

private:
    void init_or_read_share_identity();

public:

    /// @sa Share_iterator
    Share_iterator begin()
    {
        return Share_iterator(*this);
    }

    /// @sa Share_iterator
    Share_iterator end()
    {
        return Share_iterator();
    }

    /// @returns file metadata given a path, null if there's no such file
    std::unique_ptr<MFile> get_file_info(const std::string& path);

    /// add new discovered file
    void insert_mfile(const MFile&);

    /// update existing file
    void update_mfile(const MFile&);


    /// starts a filesystem scan to detect file changes and checksum files that were modified.
    void scan();

    /// @returns true if there's more to do, false otherwise, meaning scan and cksum finished
    bool scan_step();

    /// @returns true if a scan is in progress
    bool scan_in_progress() const { return m_scan_in_progress; }

    bfs::path fullpath(const bfs::path& relative_to_share)
    {
        assert(relative_to_share.is_relative());
        return m_path / relative_to_share;
    }

    /// @returns number of seconds the last scan took, with a minimum of 1 second
    u32 scan_duration_s() const
    {
        return std::max(1u, static_cast<u32>(m_scan_duration_s));
    }


    // Interface for updates

    /**
     * Get updates since the given changed_by, revision pairs.
     *
     * The pairs are (peer_id, revision), the latest revisions to which the peer got updates from
     * every other peer
     */
    FrozenManifest get_updates(const std::string& peer_id, const std::map<std::string, std::string>& since = std::map<std::string, std::string>())
    {
        return FrozenManifest(peer_id, *this, since);
    }


private:
    /// @returns true if there's more to do, this does one step in the scan part
    bool fs_scan_step();

    // called once after scanning and checksumming finishes
    void on_scan_finished();

#if 0
    size_t scan_total() const { assert(0); }
    size_t scan_done() const { assert(0); }
    bool checksum_in_progress() const { assert(0); }
    size_t checksum_total() const { assert(0); }
    size_t checksum_done() const { assert(0); }
#endif

    /// actions to perform for each scanned file
    void scan_found(MFile& file);

public:
    std::string m_path;
    mpz_class m_revision;

    sqlite3pp::database m_db;
    /// path to the sqlite database of the share
    std::string m_db_path;
    sqlite3pp::command m_insert_mfile_q;
    sqlite3pp::command m_update_mfile_q;


    /********* SCAN ************/

    bool m_scan_in_progress;
    /// number of files to scan (stat) at once. We should target <= 0.5s
    size_t m_scan_batch_sz;
    std::unique_ptr<bfs::recursive_directory_iterator> m_scan_it;
    size_t m_scan_found_count;
    std::time_t m_scan_duration_s;
    sqlite3pp::query m_select_not_scan_found_q;
    sqlite3pp::command m_update_scan_found_false_q;


    /********** CKSUM *************/

    /// cksum buffer size, amount to read at once when cksumming files
    static const size_t s_cksum_block_sz = 65536;
    /**
     * number of reads to perform while calculating checksum in each step
     * total s_cksum_block_sz * m_cksum_batch_sz bytes will be read from disk before returning, target <= 0.5s
     */
    size_t m_cksum_batch_sz;

    /// query that returns the files that need to be cksummed
    sqlite3pp::query m_select_to_cksum_q;
    Checksummer m_cksummer;

    /********** SHARE IDENTITY, KEYS ***********/

    /// share id, shared publicly 32bytes
    std::string m_share_id;
    /// 16bytes
    std::string m_peer_id;

    /// pre-shared key read-write
    std::string m_psk_rw;
    /// pre-shared key read-only
    std::string m_psk_ro;
    /// pre-shared key untrusted
    std::string m_psk_untrusted;

    /// private keys 256bytes
    std::string m_pkc_rw;
    std::string m_pkc_ro;
};

/// returns a path with the last tail number of components
bfs::path get_tail(const bfs::path& path, size_t tail);


} // end ns
} // end ns
