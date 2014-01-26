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

#include "cs/message.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace cs::message;
namespace 
{
    const string ping_message = R"({"type": "ping"})";

    const string unk_message = R"({"type": "blashdaksdjkal"})";
}

BOOST_AUTO_TEST_CASE(MessageTest_01)
{
    // Given a json string we can create a message
    Message mp(ping_message);
    BOOST_CHECK(mp.type() == MType::PING);

    // Given a json string with an unrecognized message we can create a message of UNKNOWN type
    Message mi(unk_message);
    BOOST_CHECK(mi.type() == MType::UNKNOWN);
}

BOOST_AUTO_TEST_CASE(MessageTest_refine)
{
    Message m(ping_message);
    BOOST_CHECK(m.valid());
    BOOST_CHECK(m.type() == MType::PING);

    Ping ping = m.refine<Ping>();
    BOOST_CHECK(ping.valid());
    BOOST_CHECK(ping.type() == MType::PING);
 
}
