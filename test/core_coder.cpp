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

#include "cs/core/coder.hpp"
#include <boost/test/unit_test.hpp>
#include "cs/protocolstate.hpp"

using namespace std;
using namespace cs::core;
using namespace cs::core::msg;


BOOST_AUTO_TEST_CASE(MessageTest_type_keys) {
    Coder coder;

    string input = "{\
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
    cs::MsgRstate msgrstate = cs::find_message(output);
    assert(msgrstate.found);
    unique_ptr<Keys> out_msg(static_cast<Keys*>(coder.decode_msg(false, msgrstate.encoded, msgrstate.encoded_sz, "sig", 3).release()));

    BOOST_CHECK(msg->m_type == out_msg->m_type);
    BOOST_CHECK(msg->m_access == out_msg->m_access);
    BOOST_CHECK(msg->m_share_id == out_msg->m_share_id);
    BOOST_CHECK(msg->m_ro_psk == out_msg->m_ro_psk);
    BOOST_CHECK(msg->m_ro_rsa == out_msg->m_ro_rsa);
    BOOST_CHECK(msg->m_rw_public_rsa == out_msg->m_rw_public_rsa);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_manifest) {
    Coder coder;

    string input = "{\
      \"type\": \"manifest\",\
      \"peer\": \"489d80c2f2aba1ff3c7530d0768f5642\",\
      \"revision\": 379,\
      \"files\": [\
        {\
          \"path\": \"photos/img1.jpg\",\
          \"utime\": 1379220476.4512,\
          \"size\": 2387629,\
          \"mtime\": [1379220393, 323518242],\
          \"mode\": \"0664\",\
          \"sha256\": \"cf16aec13a8557cab5e5a5185691ab04f32f1e581cf0f8233be72ddeed7e7fc1\"\
        },\
        {\
          \"path\": \"photos/img2.jpg\",\
          \"utime\": 1379220468.2303,\
          \"size\": 6293123,\
          \"mtime\": [1379100421,421442491],\
          \"mode\": \"0600\",\
          \"sha256\": \"64578d0dc44b088b030ee4b258de316b5cb07fdf42b8d40050fe2635659303ed\"\
        },\
        {\
          \"path\": \"photos/img3.jpg\",\
          \"utime\": 1379489028.4324,\
          \"deleted\": true\
        }\
      ]\
    }";

    unique_ptr<Manifest> msg(static_cast<Manifest*>(coder.decode_msg(false, input.c_str(), input.size(), "sig", 3).release()));

    // Check decoding
    BOOST_CHECK(msg->m_type == MType::MANIFEST);
    BOOST_CHECK(msg->m_peer == "489d80c2f2aba1ff3c7530d0768f5642");
    BOOST_CHECK(msg->m_revision == 379);
    BOOST_CHECK(msg->m_files.size() == 3);

    // Check encoding
    std::string output = coder.encode_msg(*msg.get());
    unique_ptr<Manifest> out_msg(static_cast<Manifest*>(coder.decode_msg(false, output.c_str(), output.size(), "sig", 3).release()));

    BOOST_CHECK(msg->m_type == out_msg->m_type);
    BOOST_CHECK(msg->m_peer == out_msg->m_peer);
    BOOST_CHECK(msg->m_revision == out_msg->m_revision);
    BOOST_CHECK(msg->m_files.size() == out_msg->m_files.size());
    for (size_t i = 0; i < msg->m_files.size(); i++) {
        BOOST_CHECK(msg->m_files[i].m_path == out_msg->m_files[i].m_path);
        BOOST_CHECK(msg->m_files[i].m_utime == out_msg->m_files[i].m_utime);
        BOOST_CHECK(msg->m_files[i].m_size == out_msg->m_files[i].m_size);
        BOOST_CHECK(msg->m_files[i].m_mtime == out_msg->m_files[i].m_mtime);
        BOOST_CHECK(msg->m_files[i].m_mode == out_msg->m_files[i].m_mode);
        BOOST_CHECK(msg->m_files[i].m_sha256 == out_msg->m_files[i].m_sha256);
    }
}
