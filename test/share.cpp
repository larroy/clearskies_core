/*
 *  This file is part of clearskies_core.

 *  clearskies_core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  clearskies_core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with clearskies_core.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cs/share.hpp"
#include "cs/boost_fs_fwd.hpp"
#include <boost/test/unit_test.hpp>
#include <utility>

using namespace std;
using namespace cs::share;

enum 
{
    TMPDIR,
    DBPATH,
};

tuple<string, string> create_tmpdir_dbpath()
{
    bfs::path tmpdir = bfs::temp_directory_path();
    tmpdir /= bfs::unique_path("%%%%-%%%%-%%%%-%%%%");
    cout << tmpdir.string() << endl;
    assert(! bfs::exists(tmpdir));
    bfs::create_directory(tmpdir);
    assert(bfs::exists(tmpdir));
    bfs::path dbfile(tmpdir.string() + "__cs.db");
    assert(! bfs::exists(dbfile));
    return make_tuple(tmpdir.string(), dbfile.string());
}

BOOST_AUTO_TEST_CASE(Share_test_01)
{
    auto tmpdir_dbpath = create_tmpdir_dbpath();
    Share share(get<TMPDIR>(tmpdir_dbpath));
    share.scan_thread();
}
