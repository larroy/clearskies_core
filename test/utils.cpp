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
#include "cs/utils.hpp"

using namespace std;

using namespace cs;
using namespace cs::utils;

BOOST_AUTO_TEST_CASE(isotime_test)
{
    //BOOST_CHECK_EQUAL(isotime(1391732661), "2014-02-07T00:24:21Z");
    BOOST_CHECK_EQUAL(isotime(1391732661), "20140207T002421Z");
    //BOOST_CHECK_EQUAL(isotime_from_str("2014-02-07T00:24:21Z"), 1391732661);
    BOOST_CHECK_EQUAL(isotime_from_str("20140207T002421Z"), 1391732661);
}


BOOST_AUTO_TEST_CASE(hex_to_binary_text)
{
    const u64 x = 0xb39acfec72f75212;
    const string hex = bin_to_hex(&x, sizeof(uint64));
    const string bin = hex_to_bin<std::string>(hex);
    const u64 rx = *reinterpret_cast<const uint64*>(bin.data());
    BOOST_CHECK_EQUAL(rx, x);
}

BOOST_AUTO_TEST_CASE(random_bytes_test)
{
    const auto r1 = random_bytes(128);
    const auto r2 = random_bytes(128);
    //cout << bin_to_hex(r1) << endl;
    //cout << bin_to_hex(r2) << endl;
    BOOST_CHECK(r1 != r2);
}


BOOST_AUTO_TEST_CASE(ScopeGuardTest)
{
    int a = 0;
    {
        ScopeGuard guard = make_scope_guard([&]{ a++; });
        BOOST_CHECK_EQUAL(a, 0);
    }
    BOOST_CHECK_EQUAL(a, 1);
    {
        auto guard = make_scope_guard([&]{ a++; });
        BOOST_CHECK_EQUAL(a, 1);
    }
    BOOST_CHECK_EQUAL(a, 2);
    {
        ScopeGuard guard = make_scope_guard([&]{ a++; });
        guard.disable();
        BOOST_CHECK_EQUAL(a, 2);
    }
    BOOST_CHECK_EQUAL(a, 2);
    {
        for (int i = 0; i < 3; i++ ) {
            ScopeGuard guard = make_scope_guard([&]{ a++; });
            if ( i == 1 )
                continue;
            ScopeGuard guard2 = make_scope_guard([&]{ a++; });
        }
    }
    BOOST_CHECK_EQUAL(a, 7);
}

BOOST_AUTO_TEST_CASE(Tmpdir_test)
{
    bfs::path p;
    {
        Tmpdir tmpdir;
        assert(bfs::exists(tmpdir.path));
        p = tmpdir.path;
    }
    assert(! bfs::exists(p));
}

BOOST_AUTO_TEST_CASE(create_file_read_file_test)
{
    Tmpdir tmp;
    const auto fpath = tmp.path / "testfile";
    const string content = "abrakadabra";
    create_file(fpath, content);
    const auto rcontent = read_file(fpath);
    BOOST_CHECK_EQUAL(content, rcontent);
}
