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
 */
class Share
{
public:
    Share(const std::string& share_path, const std::string& dbpath = ":memory:");
    void initialize_tables();


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
    void checksum_thread() {};

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

bfs::path get_tail(const bfs::path& path, size_t tail);


} // end ns
} // end ns
