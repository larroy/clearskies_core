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
#include "cs/messagecoder.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace cs::message;

BOOST_AUTO_TEST_CASE(MessageTest_type_from_str) {
    BOOST_CHECK(mtype_from_string("unknown") == MType::UNKNOWN);
    BOOST_CHECK(mtype_from_string("__internal_start") == MType::INTERNAL_START);
    BOOST_CHECK(mtype_from_string("ping") == MType::PING);
    BOOST_CHECK(mtype_from_string("greeting") == MType::GREETING);
    BOOST_CHECK(mtype_from_string("start") == MType::START);
    BOOST_CHECK(mtype_from_string("cannot_start") == MType::CANNOT_START);
    BOOST_CHECK(mtype_from_string("starttls") == MType::STARTTLS);
    BOOST_CHECK(mtype_from_string("identity") == MType::IDENTITY);
    BOOST_CHECK(mtype_from_string("keys") == MType::KEYS);
    BOOST_CHECK(mtype_from_string("keys_acknowledgment") == MType::KEYS_ACKNOWLEDGMENT);
    BOOST_CHECK(mtype_from_string("manifest") == MType::MANIFEST);
    BOOST_CHECK(mtype_from_string("get_manifest") == MType::GET_MANIFEST);
    BOOST_CHECK(mtype_from_string("get") == MType::GET);
    BOOST_CHECK(mtype_from_string("file_data") == MType::FILE_DATA);
    BOOST_CHECK(mtype_from_string("update") == MType::UPDATE);
    BOOST_CHECK(mtype_from_string("move") == MType::MOVE);
    BOOST_CHECK(mtype_from_string("aarsrasrasa") == MType::UNKNOWN);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_to_str) {
    BOOST_CHECK(mtype_to_string(MType::UNKNOWN) == "unknown");
    BOOST_CHECK(mtype_to_string(MType::INTERNAL_START) == "__internal_start");
    BOOST_CHECK(mtype_to_string(MType::PING) == "ping");
    BOOST_CHECK(mtype_to_string(MType::GREETING) == "greeting");
    BOOST_CHECK(mtype_to_string(MType::START) == "start");
    BOOST_CHECK(mtype_to_string(MType::CANNOT_START) == "cannot_start");
    BOOST_CHECK(mtype_to_string(MType::STARTTLS) == "starttls");
    BOOST_CHECK(mtype_to_string(MType::IDENTITY) == "identity");
    BOOST_CHECK(mtype_to_string(MType::KEYS) == "keys");
    BOOST_CHECK(mtype_to_string(MType::KEYS_ACKNOWLEDGMENT) == "keys_acknowledgment");
    BOOST_CHECK(mtype_to_string(MType::MANIFEST) == "manifest");
    BOOST_CHECK(mtype_to_string(MType::GET_MANIFEST) == "get_manifest");
    BOOST_CHECK(mtype_to_string(MType::GET) == "get");
    BOOST_CHECK(mtype_to_string(MType::FILE_DATA) == "file_data");
    BOOST_CHECK(mtype_to_string(MType::UPDATE) == "update");
    BOOST_CHECK(mtype_to_string(MType::MOVE) == "move");
}

BOOST_AUTO_TEST_CASE(MessageTest_access_from_str) {
    BOOST_CHECK(maccess_from_string("unknown") == MAccess::UNKNOWN);
    BOOST_CHECK(maccess_from_string("read_only") == MAccess::READ_ONLY);
    BOOST_CHECK(maccess_from_string("read_write") == MAccess::READ_WRITE);
}

BOOST_AUTO_TEST_CASE(MessageTest_access_to_str) {
    BOOST_CHECK(maccess_to_string(MAccess::UNKNOWN) == "unknown");
    BOOST_CHECK(maccess_to_string(MAccess::READ_ONLY) == "read_only");
    BOOST_CHECK(maccess_to_string(MAccess::READ_WRITE) == "read_write");
}

BOOST_AUTO_TEST_CASE(MessageTest_type_keys) {
    Coder coder;

    string input = "s347:{\
      \"type\": \"keys\",\
      \"access\": \"read_only\",\
      \"share_id\": \"2bd01bbb634ec221f916e176cd2c7c6c2fa04e641c494979613d3485defd7d18\",\
      \"read_only\": {\
        \"psk\": \"b699049ce1f453628117e8ba6ee75f42\",\
        \"rsa\": \"-----BEGIN RSA PRIVATE KEY-----\\nMIIJKAIBAAKCAgEA4Zu1XDLoHf...TE KEY-----\\n\"\
      },\
      \"read_write\": {\
        \"public_rsa\": \"-----BEGIN RSA PUBLIC KEY-----\\nMIIBgjAcBgoqhkiG9w0BDAEDMA4E...\"\
      }\
    }";

    unique_ptr<Keys> msg(static_cast<Keys*>(coder.decode_msg(false, input.c_str(), input.size(), "sig", 3).release()));

    // Check decoding
    BOOST_CHECK(msg->m_type == MType::KEYS);
    BOOST_CHECK(msg->m_access == MAccess::READ_ONLY);
    BOOST_CHECK(msg->m_share_id == "2bd01bbb634ec221f916e176cd2c7c6c2fa04e641c494979613d3485defd7d18");
    BOOST_CHECK(msg->m_ro_psk == "b699049ce1f453628117e8ba6ee75f42");
    BOOST_CHECK(msg->m_ro_rsa == "-----BEGIN RSA PRIVATE KEY-----\nMIIJKAIBAAKCAgEA4Zu1XDLoHf...TE KEY-----\n");
    BOOST_CHECK(msg->m_rw_public_rsa == "-----BEGIN RSA PUBLIC KEY-----\nMIIBgjAcBgoqhkiG9w0BDAEDMA4E...");

    // Check encoding
    std::string output = coder.encode_msg(*msg.get());
    unique_ptr<Keys> out_msg(static_cast<Keys*>(coder.decode_msg(false, input.c_str(), input.size(), "sig", 3).release()));

    BOOST_CHECK(msg->m_type == out_msg->m_type);
    BOOST_CHECK(msg->m_access == out_msg->m_access);
    BOOST_CHECK(msg->m_share_id == out_msg->m_share_id);
    BOOST_CHECK(msg->m_ro_psk == out_msg->m_ro_psk);
    BOOST_CHECK(msg->m_ro_rsa == out_msg->m_ro_rsa);
    BOOST_CHECK(msg->m_rw_public_rsa == out_msg->m_rw_public_rsa);
}
