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


#define REDIRECT_COUT streambuf* old_cout_rdbuf__ = cout.rdbuf(); ostringstream os; cout.rdbuf(os.rdbuf());
#define REDIRECT_COUT_END cout.rdbuf(old_cout_rdbuf__);

#define REDIRECT_CLOG streambuf* old_clog_rdbuf__ = clog.rdbuf(); ostringstream os; clog.rdbuf(os.rdbuf());
#define REDIRECT_CLOG_END clog.rdbuf(old_clog_rdbuf__);


#define REDIRECT_CERR streambuf* old_cerr_rdbuf__ = cerr.rdbuf(); ostringstream os; cerr.rdbuf(os.rdbuf());
#define REDIRECT_CERR_END cerr.rdbuf(old_cerr_rdbuf__);

#include "cs/boost_fs_fwd.hpp"
#include "cs/utils.hpp"

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


inline void create_tree(const bfs::path& path)
{
    bfs::create_directory(path / "a");
    bfs::path aaa = path / "a" / "aa";
    bfs::create_directory(aaa);
    bfs::path aaaf = aaa / "f";
    bfs::ofstream aaaf_os(aaaf);
    aaaf_os << "aaaf content " << std::endl;

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

