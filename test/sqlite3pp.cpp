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
#include "sqlite3pp/sqlite3pp.hpp"
#include <cstdint>

using namespace std;
using namespace sqlite3pp;


BOOST_AUTO_TEST_CASE(test_get)
{
    auto db(make_shared<database>(":memory:"));
    command(db, "CREATE TABLE test (i INTEGER, t TEXT, b BLOB)").execute();

    uint64_t i = std::numeric_limits<uint64_t>::max();
    (void) i;
    string t;
    t.push_back(0);
    t.push_back(1);
    t.push_back(2);
    t.push_back('a');
    t.push_back(0xc3);
    t.push_back(0xa1);

    string b;
    t.push_back(0);
    t.push_back(1);
    t.push_back(2);
    t.push_back('b');
    t.push_back(0xc3);
    t.push_back(0xa1);

    command ins(db, "INSERT INTO test (i,t,b) VALUES (:v1, :v2, :v3)");
    ins.bind(":v1", i).bind(":v2", t).bind(":v3", b.c_str(), b.size());
    ins.execute();

    query q(db, "SELECT * FROM test");
    BOOST_CHECK_EQUAL(q.column_count(), 3);
    for (const auto& r: q)
    {
        BOOST_CHECK_EQUAL(r.column_count(), 3);
        int ri = r.get<int>(0);
        BOOST_CHECK_EQUAL(i, ri);
        string rt = r.get<string>(1);
        BOOST_CHECK_EQUAL(t,rt);
        string rb = r.get<string>(2);
        BOOST_CHECK_EQUAL(b,rb);
    }
}

