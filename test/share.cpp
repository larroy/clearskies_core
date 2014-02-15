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
#include "cs/utils.hpp"

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
    Tmpdir():
        tmpdir(cs::utils::tmpdir())
        , dbpath(tmpdir.string() + "__cs.db")
    {
        // cout << tmpdir.string() << endl;
        assert(! bfs::exists(tmpdir));
        bfs::create_directory(tmpdir);
        assert(bfs::exists(tmpdir));
        assert(! bfs::exists(dbpath));
    }
    ~Tmpdir()
    {
        bfs::remove_all(tmpdir);
        bfs::remove(dbpath);
    }
    bfs::path tmpdir;
    bfs::path dbpath;
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
    Share share(tmp.tmpdir.string());
    create_tree(tmp.tmpdir);
}

BOOST_AUTO_TEST_CASE(tail_test)
{
    BOOST_CHECK_EQUAL(get_tail(bfs::path("a/b/c"), 2).string(), "b/c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("a/b/c"), 1).string(), "c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c"), 4).string(), "/a/b/c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c/"), 1).string(), ".");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c/d"), 1).string(), "d");
}

BOOST_AUTO_TEST_CASE(share_save_mfile)
{
    MFile f;
    f.path = "omg/a/path";
    f.mtime = "12392";
    f.size = 69;
    f.mode = 01777;

    Tmpdir tmp;
    Share share(tmp.tmpdir.string());

    auto f_none = share.get_file_info("argsgs");
    BOOST_CHECK(! f_none);

    share.save_mfile(f);

    f_none = share.get_file_info("argsgs");
    BOOST_CHECK(! f_none);

    auto f_ = share.get_file_info(f.path);
    BOOST_CHECK(f_);
    BOOST_CHECK_EQUAL(f_->path, f.path);
    BOOST_CHECK_EQUAL(f_->mtime, f.mtime);
    BOOST_CHECK_EQUAL(f_->size, f.size);
    BOOST_CHECK_EQUAL(f_->mode, f.mode);
    BOOST_CHECK_EQUAL(f_->sha256, f.sha256);
    BOOST_CHECK_EQUAL(f_->deleted, f.deleted);
}



BOOST_AUTO_TEST_CASE(share_checksum_thread)
{
    Tmpdir tmp;
    Share share(tmp.tmpdir.string());
    create_tree(tmp.tmpdir);

    for (const auto& file: share)
        BOOST_CHECK(file.sha256.empty());

    share.scan();
    while(share.scan_step());

    size_t nfiles = 0;
    for (const auto& file: share)
    {
        //cout << file.path << endl;
        ++nfiles;
        BOOST_CHECK(! file.sha256.empty());
        //cout << file.sha256 << endl;
        //cout << file.mtime << endl;
    }
    BOOST_CHECK_EQUAL(nfiles, 3);
}
