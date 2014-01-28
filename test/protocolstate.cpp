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

#include "cs/protocolstate.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>

using namespace std;
using namespace cs::message;
using namespace cs::protocol;

class ProtocolTest: public ProtocolState
{
public:
    ProtocolTest(): 
        ProtocolState()
        , m_messages()
        , m_payload()
        , m_payload_end()
    {
    }
    void handle_message(const Message& m) override
    {
        m_messages.emplace_back(m);
    }

    void handle_payload(const char* data, size_t len) override
    {
        m_payload.append(data, len);
    }

    void handle_payload_end() override
    {
        m_payload_end = true;
    }
    vector<Message> m_messages;
    string m_payload;
    bool m_payload_end;
};

BOOST_AUTO_TEST_CASE(find_messsage_test)
{
    string buff = "some garbage\n";
    MsgFound found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(found.garbage);
    BOOST_CHECK(found.json.empty());
    BOOST_CHECK(! found.prefix);
    BOOST_CHECK_EQUAL(&*found.end, &buff[13]);

    buff = "";
    found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK(found.json.empty());
    BOOST_CHECK(! found.prefix);
    BOOST_CHECK_EQUAL(&*found.end, &buff[0]);

    buff = "\n\n";
    found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(found.garbage);
    BOOST_CHECK(found.json.empty());
    BOOST_CHECK(! found.prefix);
    BOOST_CHECK_EQUAL(&*found.end, &buff[1]);

    buff = "{}\n";
    found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "{}");
    BOOST_CHECK(! found.prefix);
    BOOST_CHECK_EQUAL(&*found.end, &buff[3]);

    buff = "!{}\n";
    found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "{}");
    BOOST_CHECK(found.prefix == '!');
    BOOST_CHECK_EQUAL(&*found.end, &buff[4]);
}


BOOST_AUTO_TEST_CASE(find_messsage_test_02)
{
    string buff = "${}\n";
    MsgFound found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "");
    BOOST_CHECK(found.prefix == '$');
    BOOST_CHECK_EQUAL(&*found.end, &buff[0]);

    buff.append("=123\n");
    found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "{}");
    BOOST_CHECK_EQUAL(found.signature, "=123");
    BOOST_CHECK(found.prefix == '$');
    BOOST_CHECK_EQUAL(&*found.end, &buff[9]);
}

BOOST_AUTO_TEST_CASE(find_messsage_test_03)
{
    /// partial signature read
    string buff = "${}\n";
    MsgFound found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "");
    BOOST_CHECK(found.prefix == '$');
    BOOST_CHECK_EQUAL(&*found.end, &buff[0]);

    buff.append("=1");
    found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "");
    BOOST_CHECK_EQUAL(found.signature, "");
    BOOST_CHECK(found.prefix == '$');
    BOOST_CHECK_EQUAL(&*found.end, &buff[0]);

    buff.append("23\n");
    found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "{}");
    BOOST_CHECK_EQUAL(found.signature, "=123");
    BOOST_CHECK(found.prefix == '$');
    BOOST_CHECK_EQUAL(&*found.end, &buff[9]);
}

BOOST_AUTO_TEST_CASE(find_messsage_test_04)
{
    string buff = "&{}\n";
    MsgFound found = find_message(buff);
    BOOST_CHECK(! found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "");
    BOOST_CHECK(found.prefix == '&');
    BOOST_CHECK_EQUAL(&*found.end, &buff[0]);

    buff.append("=123\n");
    found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, "{}");
    BOOST_CHECK_EQUAL(found.signature, "=123");
    BOOST_CHECK(found.prefix == '&');
    BOOST_CHECK_EQUAL(&*found.end, &buff[9]);
}

BOOST_AUTO_TEST_CASE(find_messsage_test_05)
{

    string buff = R"({"type": "ping"})" "\n";
    MsgFound found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, R"({"type": "ping"})");
    BOOST_CHECK(found.prefix == '\0');
    BOOST_CHECK_EQUAL(&*found.end, &buff[17]);

    buff = R"(!{"type": "ping"})" "\n";
    found = find_message(buff);
    BOOST_CHECK(found.found);
    BOOST_CHECK(! found.garbage);
    BOOST_CHECK_EQUAL(found.json, R"({"type": "ping"})");
    BOOST_CHECK(found.prefix == '!');
    BOOST_CHECK_EQUAL(&*found.end, &buff[18]);

}

BOOST_AUTO_TEST_CASE(ProtocolStateTest_01)
{
    ProtocolTest proto;
    string ping = R"({"type": "ping"})" "\n";
    proto.input(ping);
    BOOST_CHECK_EQUAL(proto.m_messages.size(), 1u);
    const Message& m = proto.m_messages.at(0);
    BOOST_CHECK(m.type() == MType::PING);
}

BOOST_AUTO_TEST_CASE(ProtocolStateTest_02)
{
    // Given partial reads, check that the message is assembled
    ProtocolTest proto;
    proto.input(R"({"typ)");
    proto.input(R"(e": "pi)");
    proto.input(R"(ng"})");
    proto.input("\n");
    BOOST_CHECK_EQUAL(proto.m_messages.size(), 1u);
    const Message& m = proto.m_messages[0];
    BOOST_CHECK(m.type() == MType::PING);
}

BOOST_AUTO_TEST_CASE(ProtocolStateTest_payload_01)
{
    ProtocolTest proto;
    proto.input("!{}\n5\n012\n10");
    BOOST_CHECK_EQUAL(proto.m_payload, "012\n1");
    BOOST_CHECK(!proto.m_payload_end);
    proto.input("\n");
    BOOST_CHECK(proto.m_payload_end);
}

BOOST_AUTO_TEST_CASE(ProtocolStateTest_payload_02)
{
    ProtocolTest proto;
    proto.input("!{}\n5\n01");
    BOOST_CHECK(! proto.m_payload_end);
    proto.input("2\n");
    BOOST_CHECK(! proto.m_payload_end);
    proto.input("10\n");
    BOOST_CHECK_EQUAL(proto.m_payload, "012\n1");
    BOOST_CHECK(proto.m_payload_end);
}
