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

namespace cs
{
namespace share
{

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
 */
class Share
{
public:

    /**
     * Iterator to allow for(const auto& file: share) idiom
     */
    class Share_iterator: public boost::iterator_facade<Share_iterator, File, boost::single_pass_traversal_tag>
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

        File& dereference() const;

        std::unique_ptr<sqlite3pp::query> m_query;
        sqlite3pp::query::query_iterator m_query_it;
        mutable File m_file;
        mutable bool m_file_set;
    };


    Share(const std::string& share_path, const std::string& dbpath = ":memory:");
    void initialize_tables();

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

    void scan();

// FIXME: TODO, progress indicators
    bool scan_in_progress() const { assert(0); }
    size_t scan_total() const { assert(0); }
    size_t scan_done() const { assert(0); }

    bool checksum_in_progress() const { assert(0); }
    size_t checksum_total() const { assert(0); }
    size_t checksum_done() const { assert(0); }

    /// @returns file metadata given a path, null if there's no such file
    std::unique_ptr<File> get_file_info(const std::string& path);
    /// save File metadata on the Share
    void set_file_info(const File&);


// FIXME: to make non public / testable

    /// first pass, "stat"
    void scan_thread();
    /// second pass, "checksum"
    void checksum_thread();

    /// actions to perform for each scanned file
    void scan_file(File&& file);

private:
    std::thread m_scan_thread;

public:
    std::string m_path;
    sqlite3pp::database m_db;
    std::string m_db_path;

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
