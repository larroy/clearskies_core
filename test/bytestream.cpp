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
#include <iterator>
#include <algorithm>
#include <iostream>
#include "cs/utils.hpp"
#include "cs/ibytestream.hpp"
#include "cs/obytestream.hpp"

using namespace std;
using namespace cs::io;
using namespace cs;

BOOST_AUTO_TEST_CASE(Ibytestream_test_01)
{
    Obytestream ob;
    u32 v = 0xa5de4137u;
    ob.write<u32>(v);
    Ibytestream ib(ob.begin(), ob.end());
    u32 vr = ib.read<u32>();
    BOOST_CHECK_EQUAL(v, vr);
}

template<typename T>
void test_encode_decode(const size_t count)
{
    Obytestream ob;
    const auto v = utils::random_uniform_vector<T>(count);
    for (const auto& x: v)
        ob.write<T>(x);

    Ibytestream ib(ob.begin(), ob.end());
    vector<T> read;
    generate_n(back_inserter(read), count, [&] { return ib.read<T>(); });
    BOOST_CHECK_EQUAL_COLLECTIONS(v.begin(), v.end(), read.begin(), read.end());
}

BOOST_AUTO_TEST_CASE(Ibytestream_test_02)
{
    test_encode_decode<u8>(1024);
    test_encode_decode<u16>(1024);
    test_encode_decode<u32>(1024);
    test_encode_decode<u64>(1024);

    test_encode_decode<i8>(1024);
    test_encode_decode<i16>(1024);
    test_encode_decode<i32>(1024);
    test_encode_decode<i64>(1024);

}

