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


#include "cs/server.hpp"
#include "cs/core/share.hpp"
#include "test_utils.hpp"
#include "cs/core/coder.hpp"
#include "cs/core/message.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include <functional>

using namespace std;
using namespace cs::server;
using namespace cs;
using namespace cs::core;
using namespace cs::core::msg;

/**
 * A local server
 */
class CSServer: public Server
{
public:
    CSServer():
        m_out_buff()
    {
       m_server_info.m_name = "CS test server"; 
       m_server_info.m_protocol = 1;
    }

    Connection& add_connection(const std::string& name)
    {
        auto res = m_connections.emplace(name, make_unique<Connection>(m_server_info, m_shares));
        assert(res.second);
        m_out_buff.emplace(name, string());
        auto do_write = [name, this](const char* buff, size_t sz)
        {
            assert(m_out_buff[name].empty());
            m_out_buff[name] = {buff,sz};
        };
        Connection& conn = *res.first->second;
        conn.m_protocolstate.set_write_fun(do_write);
        return conn;
    }

    void receive(const std::string& connection, const core::msg::Message& m)
    {
        auto i = m_connections.find(connection);
        if (i == m_connections.end())
            throw std::runtime_error(fs("Connection: " << connection << " not found"));
        Connection& conn = *i->second;
        conn.m_protocolstate.input(Coder().encode_msg(m));
    }

    /// @returns data and notify protocol
    std::string tx_write(const std::string& connection)
    {
        auto bi = m_out_buff.find(connection);
        if (bi == m_out_buff.end())
            throw std::runtime_error(fs("Connection: " << connection << " not found"));
        string res = move(bi->second);

        auto ci = m_connections.find(connection);
        assert(ci != m_connections.end());
        Connection& conn = *ci->second;
        if (! res.empty())
            /// this triggers the next write and fills the buffer, so we can write while(! empty) to read everything
            conn.m_protocolstate.on_write_finished();
        return move(res);
    }

    std::map<std::string, std::string> m_out_buff;
};


/**
 * Just a class excite the Server acting as a Peer
 */
class Peer
{
public:
    Peer(const std::string& name, CSServer& server):
          m_name{name}
        , m_id{utils::random_bytes(16)}
        , m_messages_payload{}
        , m_payload_end{true}
        , r_server(server)
        , m_protocolstate()
        , m_coder()
    {
        m_protocolstate.m_handle_msg = bind(&Peer::handle_msg, this, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5);
        m_protocolstate.m_handle_payload = bind(&Peer::handle_payload, this, placeholders::_1, placeholders::_2);
        m_protocolstate.m_handle_payload_end = bind(&Peer::handle_payload_end, this);
    }

    /// read all the output buffers from the server
    void read_from(CSServer& server)
    {
        string server_output = server.tx_write(m_name);
        while(! server_output.empty())
        {
            m_protocolstate.input(server_output);
            server_output = server.tx_write(m_name);
        }
    }

    void send(const Message& m)
    {
        r_server.receive(m_name, m);
    }

    void handle_msg(char const* msg_encoded, size_t msg_sz, char const* signature, size_t signature_sz, bool payload)
    {
        assert(m_payload_end);
        auto msg = m_coder.decode_msg(payload, msg_encoded, msg_sz, signature, signature_sz);
        m_messages_payload.emplace_back(move(msg), string());
        m_payload_end = ! payload;
    }

    void handle_payload(const char* data, size_t len)
    {
        assert(! m_messages_payload.empty());
        m_messages_payload.back().second.append(data, len);
    }

    void handle_payload_end()
    {
        m_payload_end = true;
    }

    Message* msg(size_t pos)
    {
        return m_messages_payload.at(pos).first.get();
    }

    const std::string& payload(size_t pos)
    {
        return m_messages_payload.at(pos).second;
    }

    std::string m_name;
    std::string m_id;
    vector<pair<unique_ptr<Message>, string>> m_messages_payload;
    bool m_payload_end;
    CSServer& r_server;
    ProtocolState m_protocolstate;
    Coder m_coder;
};


BOOST_AUTO_TEST_CASE(server_test_01)
{
    Tmpdir tmp;
    create_tree(tmp.tmpdir);

    CSServer server;
    const string share_id = server.attach_share(tmp.tmpdir.string(), tmp.dbpath.string());
    Connection& connection = server.add_connection("test");
    UNUSED(connection);

    Peer peer("test", server);
    peer.send(Start{
        "cs_impl_latest_trendy_language_v1.0",
        1,
        vector<string>(),
        share_id,  // share id
        "read_write",
        utils::bin_to_hex(utils::random_bytes(16)),
        "name",
        "time"
    });
    //cs::core::share::Share& tmpshare = server.share(share_id);
    peer.read_from(server);
    BOOST_CHECK_EQUAL(peer.m_messages_payload.size(), 1u);
    BOOST_CHECK(dynamic_cast<Go*>(peer.msg(0)));
    peer.m_messages_payload.clear();
}


namespace {

void server_test_01_create_tree(const bfs::path& path)
{
    // Create a few files with the same content to verify get by hash functionality
    string content = "adios con los contents de este fichero";
    create_file(path / "a0", content);
    create_file(path / "a1", content);
    create_file(path / "wow" / "a0", "a");
    create_file(path / "wowa" / "a1", content);

    // different content
    create_file(path / "wowa" / "b2", "b2");
}

Peer init_peer(const std::string& name, CSServer& server, const std::string& share_id)
{
    Peer peer(name, server);
    peer.send(Start{"CS_CORE v0.1", 1, vector<string>(), share_id, "read_write", utils::bin_to_hex(utils::random_bytes(16)), "name", "time"});
    return peer;
}

}


BOOST_AUTO_TEST_CASE(cs_send_file)
{
    using namespace cs::core::share;
    Tmpdir tmp;
    server_test_01_create_tree(tmp.tmpdir);

    CSServer server;
    const string share_id = server.attach_share(tmp.tmpdir.string(), tmp.dbpath.string());
    server.add_connection("test");
    auto& share = server.share(share_id);
    share.fullscan();

    vector<share::MFile> manifest;
    for (const auto& file: share)
        manifest.emplace_back(file);

    BOOST_CHECK_EQUAL(manifest.size(), 5u);

    Peer peer = init_peer("test", server, share_id);
    peer.read_from(server);

    peer.send(Get(manifest[0].checksum));
    peer.read_from(server);

    BOOST_CHECK_EQUAL(peer.m_messages_payload.size(), 2u);


    const FileData* file_data = 0;
    BOOST_CHECK_NO_THROW(file_data = dynamic_cast<const FileData*>(peer.msg(1)));
    BOOST_CHECK_EQUAL(file_data->m_payload, true);
    BOOST_CHECK_EQUAL(peer.payload(1).size(), 38u);
    //BOOST_CHECK_EQUAL(peer.m_messages_payload.at(2).second, cs::utils::read_file(share.fullpath(file_data->m_paths.at(0))));
    //BOOST_CHECK_EQUAL(file_data->m_paths.size(), 3u);

    // FIXME: get by content
    //peer.send()
}


