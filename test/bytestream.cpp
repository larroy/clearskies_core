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
#include "cs/ibytestream.hpp"
#include "cs/obytestream.hpp"

using namespace std;
using namespace cs::io;

BOOST_AUTO_TEST_CASE(Ibytestream_test_01)
{
    Obytestream ob;
    u32 v = 0xa5de4137u;
    ob.write(v);
    Ibytestream ib(&*ob.m_buff.begin(), &*ob.m_buff.end());
    u32 vr = ib.read<u32>();
    BOOST_CHECK_EQUAL(v, vr);
}

