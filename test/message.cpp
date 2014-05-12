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

#include "cs/core/message.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace cs::core::msg;

BOOST_AUTO_TEST_CASE(MessageTest_type_from_str)
{
    BOOST_CHECK(mtype_from_string("unknown") == MType::UNKNOWN);
    BOOST_CHECK(mtype_from_string("__internal_send_start") == MType::INTERNAL_SEND_START);
    BOOST_CHECK(mtype_from_string("ping") == MType::PING);
    BOOST_CHECK(mtype_from_string("greeting") == MType::GREETING);
    BOOST_CHECK(mtype_from_string("start") == MType::START);
    BOOST_CHECK(mtype_from_string("cannot_start") == MType::CANNOT_START);
    BOOST_CHECK(mtype_from_string("go") == MType::GO);
    BOOST_CHECK(mtype_from_string("identity") == MType::IDENTITY);
    BOOST_CHECK(mtype_from_string("keys") == MType::KEYS);
    BOOST_CHECK(mtype_from_string("keys_acknowledgment") == MType::KEYS_ACKNOWLEDGMENT);
    BOOST_CHECK(mtype_from_string("manifest") == MType::MANIFEST);
    BOOST_CHECK(mtype_from_string("get_updates") == MType::GET_UPDATES);
    BOOST_CHECK(mtype_from_string("get") == MType::GET);
    BOOST_CHECK(mtype_from_string("file_data") == MType::FILE_DATA);
    BOOST_CHECK(mtype_from_string("update") == MType::UPDATE);
    BOOST_CHECK(mtype_from_string("move") == MType::MOVE);
    BOOST_CHECK(mtype_from_string("aarsrasrasa") == MType::UNKNOWN);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_to_str)
{
    BOOST_CHECK(mtype_to_string(MType::UNKNOWN) == "unknown");
    BOOST_CHECK(mtype_to_string(MType::INTERNAL_SEND_START) == "__internal_send_start");
    BOOST_CHECK(mtype_to_string(MType::PING) == "ping");
    BOOST_CHECK(mtype_to_string(MType::GREETING) == "greeting");
    BOOST_CHECK(mtype_to_string(MType::START) == "start");
    BOOST_CHECK(mtype_to_string(MType::CANNOT_START) == "cannot_start");
    BOOST_CHECK(mtype_to_string(MType::GO) == "go");
    BOOST_CHECK(mtype_to_string(MType::IDENTITY) == "identity");
    BOOST_CHECK(mtype_to_string(MType::KEYS) == "keys");
    BOOST_CHECK(mtype_to_string(MType::KEYS_ACKNOWLEDGMENT) == "keys_acknowledgment");
    BOOST_CHECK(mtype_to_string(MType::MANIFEST) == "manifest");
    BOOST_CHECK(mtype_to_string(MType::GET_UPDATES) == "get_updates");
    BOOST_CHECK(mtype_to_string(MType::GET) == "get");
    BOOST_CHECK(mtype_to_string(MType::FILE_DATA) == "file_data");
    BOOST_CHECK(mtype_to_string(MType::UPDATE) == "update");
    BOOST_CHECK(mtype_to_string(MType::MOVE) == "move");
}

BOOST_AUTO_TEST_CASE(MessageTest_access_from_str)
{
    BOOST_CHECK(maccess_from_string("unknown") == MAccess::UNKNOWN);
    BOOST_CHECK(maccess_from_string("read_only") == MAccess::READ_ONLY);
    BOOST_CHECK(maccess_from_string("read_write") == MAccess::READ_WRITE);
}

BOOST_AUTO_TEST_CASE(MessageTest_access_to_str)
{
    BOOST_CHECK(maccess_to_string(MAccess::UNKNOWN) == "unknown");
    BOOST_CHECK(maccess_to_string(MAccess::READ_ONLY) == "read_only");
    BOOST_CHECK(maccess_to_string(MAccess::READ_WRITE) == "read_write");
}

void check_message_defaults(const Message& m, MType type)
{
    BOOST_CHECK(m.type() == type);
    BOOST_CHECK(!m.m_payload);
    BOOST_CHECK(!m.signature());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_unknown_defaults)
{
    Unknown m;
    check_message_defaults(m, MType::UNKNOWN);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_internal_send_start_defaults)
{
    InternalSendStart m;
    check_message_defaults(m, MType::INTERNAL_SEND_START);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_ping_defaults)
{
    Ping m;
    check_message_defaults(m, MType::PING);
    BOOST_CHECK(m.m_timeout == 60);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_greeting_defaults)
{
    Greeting m;
    check_message_defaults(m, MType::GREETING);
    BOOST_CHECK(m.m_software.empty());
    BOOST_CHECK(m.m_protocol.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_start_defaults)
{
    Start m;
    check_message_defaults(m, MType::START);
    BOOST_CHECK(m.m_software.empty());
    BOOST_CHECK(m.m_protocol == 0);
    BOOST_CHECK(m.m_features.empty());
    BOOST_CHECK(m.m_share_id.empty());
    BOOST_CHECK(m.m_access.empty());
    BOOST_CHECK(m.m_peer.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_cannot_start_defaults)
{
    CannotStart m;
    check_message_defaults(m, MType::CANNOT_START);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_go_defaults)
{
    Go m;
    check_message_defaults(m, MType::GO);
    BOOST_CHECK(m.m_peer.empty());
    BOOST_CHECK(m.m_access == MAccess::UNKNOWN);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_identity_defaults)
{
    Identity m;
    check_message_defaults(m, MType::IDENTITY);
    BOOST_CHECK(m.m_name.empty());
    BOOST_CHECK(m.m_time.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_keys_defaults)
{
    Keys m;
    check_message_defaults(m, MType::KEYS);
    BOOST_CHECK(m.m_access == MAccess::UNKNOWN);
    BOOST_CHECK(m.m_share_id.empty());
    BOOST_CHECK(m.m_ro_psk.empty());
    BOOST_CHECK(m.m_ro_rsa.empty());
    BOOST_CHECK(m.m_rw_public_rsa.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_keys_acknowledgment_defaults)
{
    KeysAcknowledgment m;
    check_message_defaults(m, MType::KEYS_ACKNOWLEDGMENT);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_manifest_defaults)
{
    Manifest m;
    check_message_defaults(m, MType::MANIFEST);
    BOOST_CHECK(m.m_peer.empty());
    BOOST_CHECK(m.m_revision == 0);
    BOOST_CHECK(m.m_files.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_get_updates_defaults)
{
    GetUpdates m;
    check_message_defaults(m, MType::GET_UPDATES);
    BOOST_CHECK(m.m_revision == 0);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_current_defaults)
{
    Current m;
    check_message_defaults(m, MType::CURRENT);
}

BOOST_AUTO_TEST_CASE(MessageTest_type_get_defaults)
{
    Get m;
    check_message_defaults(m, MType::GET);
    BOOST_CHECK(m.m_checksum.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_file_data_defaults)
{
    FileData m;
    BOOST_CHECK(m.m_payload);
    BOOST_CHECK(!m.signature());
    BOOST_CHECK(m.m_paths.empty());
    BOOST_CHECK(m.m_range.empty());
}

BOOST_AUTO_TEST_CASE(MessageTest_type_file_modified_defaults)
{
    FileModified m;
    BOOST_CHECK(!m.m_payload);
    BOOST_CHECK(!m.signature());
}


BOOST_AUTO_TEST_CASE(MessageTest_type_update_defaults)
{
    Update m;
    check_message_defaults(m, MType::UPDATE);
    BOOST_CHECK(m.m_revision == 0);
    BOOST_CHECK(m.m_file.m_path.empty()); // TODO: Check MFile is default
}

BOOST_AUTO_TEST_CASE(MessageTest_type_move_defaults)
{
    Move m;
    check_message_defaults(m, MType::MOVE);
    BOOST_CHECK(m.m_revision == 0);
    BOOST_CHECK(m.m_source.empty());
    BOOST_CHECK(m.m_destination.m_path.empty()); // TODO: Check MFile is default
}


BOOST_AUTO_TEST_CASE(MessageTest_Start_operator)
{
    BOOST_CHECK(Start("a", 1, vector<string>(), "asdas", "read_write", "1231") ==
        Start("a", 1, vector<string>(), "asdas", "read_write", "1231"));

    BOOST_CHECK(Start("a", 1, vector<string>(), "asdas", "read_writes", "1231") !=

        Start("a", 1, vector<string>(), "asdas", "read_write", "1231"));
}

BOOST_AUTO_TEST_CASE(MessageTest_Go_operator)
{
    BOOST_CHECK(Go("b", MAccess::READ_ONLY) ==
        Go("b", MAccess::READ_ONLY));

    BOOST_CHECK(Go("b", MAccess::READ_ONLY) !=
        Go("a", MAccess::READ_ONLY));

    BOOST_CHECK(Go("a", MAccess::READ_ONLY) !=
        Go("a", MAccess::READ_WRITE));
}
