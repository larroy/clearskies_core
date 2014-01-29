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
    const string notype_message = R"({"blah": 1234, "c": "ahj"})";
    const string empty_message = "";
    const string badjson1_message = R"({"what)";
    const string badjson2_message = R"(1235})";
}

BOOST_AUTO_TEST_CASE(MessageTest_01)
{
    // Given a json string we can create a message
    BOOST_CHECK(Message(ping_message).type() == MType::PING);
    // Given a json string with an unrecognized message we can create a message of UNKNOWN type
    BOOST_CHECK(Message(unk_message).type() == MType::UNKNOWN);
    BOOST_CHECK(! Message(unk_message).valid());
    // Given a json string without "type", we can create a message of UNKNOWN type
    BOOST_CHECK(Message(notype_message).type() == MType::UNKNOWN);
    BOOST_CHECK(! Message(notype_message).valid());

    BOOST_CHECK(Message(unk_message).type() == MType::UNKNOWN);
    BOOST_CHECK(! Message(unk_message).valid());

    BOOST_CHECK(Message(empty_message).type() == MType::UNKNOWN);
    BOOST_CHECK(! Message(empty_message).valid());

    BOOST_CHECK_THROW(Message x(badjson1_message), MessageError);
    // this is not interpreted as json
    // BOOST_CHECK_THROW(Message x(badjson2_message), MessageError);
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


BOOST_AUTO_TEST_CASE(MessageTest_serialize)
{
    Ping m;
    string s = m.serialize();
    BOOST_CHECK_EQUAL(s, R"(m28{"timeout":60,"type":"ping"})" "\n");
}
