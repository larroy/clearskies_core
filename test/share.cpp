/*
 *  This file is part of clearskies_core.

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
#include <boost/test/unit_test.hpp>
#include "cs/share.hpp"
#include "cs/boost_fs_fwd.hpp"
#include <utility>

using namespace std;
using namespace cs::share;
using cs::File;

enum
{
    TMPDIR,
    DBPATH,
};

struct Tmpdir
{
    Tmpdir()
    {
        tmpdir_ = bfs::temp_directory_path();
        tmpdir_ /= bfs::unique_path("clearskies-%%%%-%%%%-%%%%-%%%%");
        // cout << tmpdir_.string() << endl;
        assert(! bfs::exists(tmpdir_));
        bfs::create_directory(tmpdir_);
        assert(bfs::exists(tmpdir_));
        dbpath_ = (tmpdir_.string() + "__cs.db");
        assert(! bfs::exists(dbpath_));

        tmpdir = tmpdir_.string();
        dbpath = dbpath_.string();
    }
    bfs::path tmpdir_;
    bfs::path dbpath_;
    string tmpdir;
    string dbpath;
    ~Tmpdir()
    {
        remove_all(tmpdir_);
        remove_all(dbpath_);
    }
};


void create_tree(const bfs::path& path)
{
    bfs::create_directory(path / "a");
    bfs::path aaa = path / "a" / "aa";
    bfs::create_directory(aaa);
    bfs::path aaaf = aaa / "f";
    bfs::ofstream aaaf_os(aaaf);
    aaaf_os << "aaaf content " << endl;

    bfs::path aab = path / "a" / "ab";
    bfs::create_directory(aab);
    bfs::path aabf = aab / "aabf";
    bfs::ofstream aabf_os(aabf);

    bfs::create_directory(path / "a" / "ac");
    bfs::path b = path / "b";
    bfs::create_directory(b);

    bfs::ofstream bf_os(b / "f");

    bfs::create_directory(path / "c");
}

BOOST_AUTO_TEST_CASE(Share_test_01)
{
    Tmpdir tmp;
    Share share(tmp.tmpdir);
    create_tree(tmp.tmpdir);
    share.scan_thread();
}

BOOST_AUTO_TEST_CASE(tail_test)
{
    BOOST_CHECK_EQUAL(get_tail(bfs::path("a/b/c"), 2).string(), "b/c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("a/b/c"), 1).string(), "c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c"), 4).string(), "/a/b/c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c/"), 1).string(), ".");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c/d"), 1).string(), "d");
}

BOOST_AUTO_TEST_CASE(share_set_file_info)
{
    File f;
    f.path = "omg/a/path";
    f.utime = "12391";
    f.mtime = "12392";
    f.size = 69;
    f.mode = 01777;

    Tmpdir tmp;
    Share share(tmp.tmpdir);

    auto f_none = share.get_file_info("argsgs");
    BOOST_CHECK(! f_none);

    share.set_file_info(f);

    f_none = share.get_file_info("argsgs");
    BOOST_CHECK(! f_none);

    auto f_ = share.get_file_info(f.path);
    BOOST_CHECK(f_);
    BOOST_CHECK_EQUAL(f_->path, f.path);
    BOOST_CHECK_EQUAL(f_->utime, f.utime);
    BOOST_CHECK_EQUAL(f_->mtime, f.mtime);
    BOOST_CHECK_EQUAL(f_->size, f.size);
    BOOST_CHECK_EQUAL(f_->mode, f.mode);
    BOOST_CHECK_EQUAL(f_->sha256, f.sha256);
    BOOST_CHECK_EQUAL(f_->deleted, f.deleted);
}



BOOST_AUTO_TEST_CASE(share_checksum_thread)
{
    Tmpdir tmp;
    Share share(tmp.tmpdir);
    create_tree(tmp.tmpdir);
    share.scan_thread();
    for (const auto& file: share)
        BOOST_CHECK(file.sha256.empty());

    share.checksum_thread();
    size_t nfiles = 0;
    for (const auto& file: share)
    {
        //cout << file.path << endl;
        ++nfiles;
        BOOST_CHECK(! file.sha256.empty());
        //cout << file.sha256 << endl;
    }
    BOOST_CHECK_EQUAL(nfiles, 3);
}
