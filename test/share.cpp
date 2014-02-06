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

struct Tmpdir
{
    Tmpdir()
    {
        tmpdir_ = bfs::temp_directory_path();
        tmpdir_ /= bfs::unique_path("clearskies-%%%%-%%%%-%%%%-%%%%");
        cout << tmpdir_.string() << endl;
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

BOOST_AUTO_TEST_CASE(Share_test_01)
{
    Tmpdir tmp;
    Share share(tmp.tmpdir);
    share.scan_thread();
}
