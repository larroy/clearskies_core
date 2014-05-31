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
#include "cs/vclock.hpp"

using namespace cs;
using namespace std;

BOOST_AUTO_TEST_CASE(vclock_test_01)
{
    Vclock paren;
    Vclock desc;

    paren.increment("a");

    desc.increment("a");
    desc.increment("a");
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("b");
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("b");
    paren.increment("b");
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("b");
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("a");
    desc.increment("c");
    BOOST_CHECK(desc.is_descendant(paren));
}

BOOST_AUTO_TEST_CASE(vclock_test_02)
{
    Vclock paren;
    Vclock desc;

    desc.increment("a", std::numeric_limits<u32>::max() / 2 - 1);
    BOOST_CHECK(desc.is_descendant(paren));


    desc.increment("a", std::numeric_limits<u32>::max() / 2 + 3);
    paren.increment("a", std::numeric_limits<u32>::max() - 1);
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("a");
    BOOST_CHECK(desc.is_descendant(paren));
}

BOOST_AUTO_TEST_CASE(vclock_test_03)
{
    Vclock paren;
    Vclock desc;

    desc.increment("a", std::numeric_limits<u32>::max() / 2 - 1);
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("a", 3);
    paren.increment("a", 3);
    BOOST_CHECK(desc.is_descendant(paren));

    desc.increment("a", 3);
    BOOST_CHECK(desc.is_descendant(paren));

}

BOOST_AUTO_TEST_CASE(vclock_values)
{
    Vclock x;

    x.increment("A");
    x.increment("B");
    x.increment("C", 2u);

    auto values = x.get_values();
    BOOST_CHECK_EQUAL(values.size(), 3u);

    BOOST_CHECK_EQUAL(values["A"], 1u);
    BOOST_CHECK_EQUAL(values["A"], 1u);
    BOOST_CHECK_EQUAL(values["C"], 2u);
}

BOOST_AUTO_TEST_CASE(vclock_construction)
{
    map<string, u64> vs;
    vs["A"] = 5;
    vs["B"] = 3;

    Vclock x(vs);
    BOOST_CHECK_EQUAL(x["A"], 5);
    BOOST_CHECK_EQUAL(x["B"], 3);
    BOOST_CHECK(x.get_values() == vs);
}

BOOST_AUTO_TEST_CASE(vclock_json_1)
{
    Vclock x;

    x.increment("A");
    x.increment("B");
    x.increment("C", 2u);

    const string s = vclock_to_json(x);
    Vclock x_ = vclock_from_json(s);
    BOOST_CHECK(x_ == x);

}
