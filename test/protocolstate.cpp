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

#include "cs/protocolstate.hpp"
#include "cs/core/coder.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>

using namespace std;
using namespace cs;
using namespace cs::core;
using namespace cs::core::msg;

// FIXME
#if 0
class ProtocolTest: public ProtocolState
{
public:
    ProtocolTest():
        ProtocolState()
        , m_messages()
        , m_payload()
        , m_payload_end()
        , m_msg_garbage_cb([](const string&){})
        , m_pl_garbage_cb([](const string&){})
    {
    }

    void handle_message(unique_ptr<Message> msg) override
    {
        m_messages.emplace_back(move(msg));
    }

    void handle_payload(const char* data, size_t len) override
    {
        m_payload.append(data, len);
    }

    void handle_payload_end() override
    {
        m_payload_end = true;
    }

    void handle_msg_garbage(const std::string& buff) override
    {
        m_msg_garbage_cb(buff);
        cout << "handle_msg_garbage: " << buff << endl;
    }

    void handle_pl_garbage(const std::string& buff) override
    {
        m_pl_garbage_cb(buff);
        cout << "handle_pl_garbage: " << buff << endl;
    }



    vector<unique_ptr<Message>> m_messages;
    string m_payload;
    bool m_payload_end;
    std::function<void(const std::string&)> m_msg_garbage_cb;
    std::function<void(const std::string&)> m_pl_garbage_cb;
};
#endif

BOOST_AUTO_TEST_CASE(find_messsage_test_01)
{
    string buff = "som: e garbage\n";
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(! mrs.found);
    BOOST_CHECK(mrs.garbage);
    BOOST_CHECK(! mrs.has_signature());
}

BOOST_AUTO_TEST_CASE(find_messsage_test_02)
{
    string buff = "m123981029830912830918203981092830912830918092380918309810923809182309\n";
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(! mrs.found);
    BOOST_CHECK(mrs.garbage);
}

BOOST_AUTO_TEST_CASE(find_messsage_test_03)
{
    string buff("m\x00\x00\x00\x02:{}", 8);
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK_EQUAL(mrs.prefix, 'm');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), "{}");
    BOOST_CHECK_EQUAL(mrs.encoded_sz, 2u);
    BOOST_CHECK_EQUAL(mrs.enc_sig_sz, 8);
    BOOST_CHECK(! mrs.has_signature());
}

BOOST_AUTO_TEST_CASE(find_messsage_test_04)
{
    string buff("!\x00\x00\x00\x07:{jsonz}", 13);
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK(mrs.payload());
    BOOST_CHECK_EQUAL(mrs.prefix, '!');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), "{jsonz}");
    BOOST_CHECK_EQUAL(mrs.encoded_sz, 7u);
    BOOST_CHECK_EQUAL(mrs.enc_sig_sz, 13u);
}

BOOST_AUTO_TEST_CASE(find_messsage_test_05)
{
    string buff("!\x00\x00\x00\x07:{jsonz}\x00\x00\x00\x05:payld", 23);
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK(mrs.payload());
    BOOST_CHECK_EQUAL(mrs.prefix, '!');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), "{jsonz}");
    BOOST_CHECK_EQUAL(mrs.encoded_sz, 7u);
    BOOST_CHECK(! mrs.has_signature());
}

BOOST_AUTO_TEST_CASE(find_messsage_test_06)
{
    string buff("s\x00\x00\x00\x07:{jsonz}\x00\x00\x00\x05:signz", 23);
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK(! mrs.payload());
    BOOST_CHECK_EQUAL(mrs.prefix, 's');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), "{jsonz}");
    BOOST_CHECK_EQUAL(mrs.encoded_sz, 7u);
    BOOST_CHECK(mrs.has_signature());
    BOOST_CHECK_EQUAL(string(mrs.signature, mrs.signature_sz), "signz");
    BOOST_CHECK_EQUAL(mrs.signature_sz, 5u);
}


BOOST_AUTO_TEST_CASE(find_messsage_test_07)
{
    string buff("$\x00\x00\x00\x07:{jsonz}\x00\x00\x00\x05:signz\x00\x00\x00\x05:payld", 34);
    MsgRstate mrs = find_message(buff);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK(mrs.payload());
    BOOST_CHECK(mrs.has_signature());
    BOOST_CHECK_EQUAL(mrs.prefix, '$');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), "{jsonz}");
    BOOST_CHECK_EQUAL(string(mrs.signature, mrs.signature_sz), "signz");
    BOOST_CHECK_EQUAL(mrs.encoded_sz, 7u);
}


BOOST_AUTO_TEST_CASE(messagecoder_test)
{
    Coder coder;
    string coded = coder.encode_msg(Ping());
    MsgRstate mrs = find_message(coded);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK(! mrs.payload());
    BOOST_CHECK_EQUAL(mrs.prefix, 'm');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), R"({"timeout":60,"type":"ping"})");


    Ping msg;
    msg.m_timeout = 10;
    coded = coder.encode_msg(msg);
    mrs = find_message(coded);
    BOOST_CHECK(mrs.found);
    BOOST_CHECK(! mrs.garbage);
    BOOST_CHECK(! mrs.payload());
    BOOST_CHECK_EQUAL(mrs.prefix, 'm');
    BOOST_CHECK_EQUAL(string(mrs.encoded, mrs.encoded_sz), R"({"timeout":10,"type":"ping"})");

}

#if 0
BOOST_AUTO_TEST_CASE(Protocol_test_01)
{
    Coder coder;
    ProtocolTest proto;
    string coded = coder.encode_msg(Greeting());
    for (const auto x: coded)
        proto.input(string(1, x));

    BOOST_CHECK_EQUAL(proto.m_messages.size(), 1u);
    BOOST_CHECK(typeid(*proto.m_messages.at(0)) == typeid(Greeting));
}

BOOST_AUTO_TEST_CASE(Protocol_test_payload)
{
    Coder coder;
    ProtocolTest proto;
    Greeting msg;
    msg.m_payload = true;
    string coded = coder.encode_msg(msg);
    coded.append("3\n1230\n");
    for (const auto x: coded)
        proto.input(string(1, x));

    BOOST_CHECK_EQUAL(proto.m_messages.size(), 1u);
    BOOST_CHECK(typeid(*proto.m_messages.at(0)) == typeid(Greeting));
    BOOST_CHECK_EQUAL(proto.m_payload, "123");

}

BOOST_AUTO_TEST_CASE(Protocol_test_signature)
{
    Coder coder;
    ProtocolTest proto;
    Greeting msg;
    msg.m_signature = "by me";
    string coded = coder.encode_msg(msg);

    for (const auto x: coded)
        proto.input(string(1, x));
    //proto.input(coded);

    //cout << coded << endl;
    BOOST_CHECK_EQUAL(proto.m_messages.size(), 1u);
    BOOST_CHECK(typeid(*proto.m_messages.at(0)) == typeid(Greeting));
    const auto & rec = *proto.m_messages.at(0);

    BOOST_CHECK(rec.signature());
    BOOST_CHECK_EQUAL(rec.m_signature, "by me");
}

#endif




#if 0
    buff = "";
    mrs = find_message(buff);
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

BOOST_AUTO_TEST_CASE(ProtocolStateTest_garbage)
{
    ProtocolTest proto;
    bool called = false;
    const string* buf;
    proto.m_pl_garbage_cb = [&](const string& buff)
    {
        called = true;
        buf = &buff;
    };
    proto.input("!{}\nasdasda\n");
    BOOST_CHECK(called);
    BOOST_CHECK_EQUAL(*buf, "");
}

BOOST_AUTO_TEST_CASE(ProtocolStateTest_garbage_02)
{
    ProtocolTest proto;
    bool called = false;
    const string* buf;
    proto.m_pl_garbage_cb = [&](const string& buff)
    {
        called = true;
        buf = &buff;
    };
    proto.input("!{}\nblashblhasblabhalbhablhaomgsoboringandnotnumeric");
    BOOST_CHECK(called);
    BOOST_CHECK_EQUAL(*buf, "");
    proto.input("{}\n");
    BOOST_CHECK_EQUAL(proto.m_messages.size(), 2);
    BOOST_CHECK(proto.m_messages[0].type() == MType::UNKNOWN);
    BOOST_CHECK(proto.m_messages[1].type() == MType::UNKNOWN);
}
#endif
