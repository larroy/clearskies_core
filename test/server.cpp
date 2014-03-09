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
#include "utils.hpp"
#include "cs/message.hpp"
#include "cs/messagecoder.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace cs::server;
using namespace cs;

/**
 * A local server
 */
class CSServer: public Server
{
public:
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
        conn.m_cs_protocol.set_write_fun(do_write);
        return conn;
    }

    void receive(const std::string& connection, const message::Message& m)
    {
        auto i = m_connections.find(connection);
        if (i == m_connections.end())
            throw std::runtime_error(fs("Connection: " << connection << " not found"));
        Connection& conn = *i->second;
        conn.m_cs_protocol.input(message::Coder().encode_msg(m));
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
            conn.m_cs_protocol.on_write_finished();
        return move(res);
    }

    std::map<std::string, std::string> m_out_buff;
};


/**
 * Just a class excite the Server acting as a Peer
 */
class Peer: public protocol::ProtocolState
{
public:
    Peer(const std::string& name):
          ProtocolState{}
        , m_name{name}
        , m_id{utils::random_bytes(16)}
        , m_messages_payload{}
        , m_payload_end{}
        , m_msg_garbage_cb{[](const string&){}}
        , m_pl_garbage_cb{[](const string&){}}
    {
    }

    /// read all the output buffers from the server
    void read_from(CSServer& server)
    {
        string server_output = server.tx_write(m_name);
        while(! server_output.empty())
        {
            input(server_output);
            server_output = server.tx_write(m_name);
        }
    }

    void handle_message(unique_ptr<message::Message> msg) override
    {
        m_messages_payload.emplace_back(move(msg), string());
    }

    void handle_payload(const char* data, size_t len) override
    {
        assert(! m_messages_payload.empty());
        m_messages_payload.back().second.append(data, len);
    }

    void handle_payload_end() override
    {
        m_payload_end = true;
    }

    void handle_msg_garbage(const std::string& buff) override
    {
        m_msg_garbage_cb(buff);
        cout << "handle_msg_garbage: " << buff << endl;
        assert(false);
    }

    void handle_pl_garbage(const std::string& buff) override
    {
        m_pl_garbage_cb(buff);
        cout << "handle_pl_garbage: " << buff << endl;
        assert(false);
    }


    std::string m_name;
    std::string m_id;
    vector<pair<unique_ptr<message::Message>, string>> m_messages_payload;
    bool m_payload_end;
    std::function<void(const std::string&)> m_msg_garbage_cb;
    std::function<void(const std::string&)> m_pl_garbage_cb;
};


BOOST_AUTO_TEST_CASE(server_test_01)
{
    using namespace cs::message;
    Tmpdir tmp;
    create_tree(tmp.tmpdir);

    CSServer server;
    const string share_id = server.attach_share(tmp.tmpdir.string(), tmp.dbpath.string());
    Connection& connection = server.add_connection("test");
    UNUSED(connection);

    Peer peer("test");
    server.receive("test", Start{
        "cs_impl_latest_trendy_language_v1.0",
        1,
        vector<string>(),
        share_id,  // share id
        "read_write",
        utils::random_bytes(16)  // peer id
    });
    share::Share& tmpshare = server.share(share_id);
    peer.read_from(server);
    BOOST_CHECK_EQUAL(peer.m_messages_payload.size(), 2u);
    BOOST_CHECK(dynamic_cast<StartTLS&>(*peer.m_messages_payload.at(0).first) == StartTLS(tmpshare.m_peer_id, MAccess::READ_WRITE));
    Identity* identity_msg{};
    BOOST_CHECK_NO_THROW(identity_msg = &dynamic_cast<Identity&>(*peer.m_messages_payload.at(1).first));
    BOOST_CHECK_EQUAL(identity_msg->m_name, server.m_server_info.m_name);
    BOOST_CHECK(identity_msg->m_time <= utils::isotime(std::time(nullptr)));

    peer.m_messages_payload.clear();


}


