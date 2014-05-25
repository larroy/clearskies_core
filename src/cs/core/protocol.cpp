/*
 *  This file is part of clearskies_core file synchronization program
 *  Copyright (C) 2014 Pedro Larroy

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
#include "protocol.hpp"
#include <cassert>
#include "boost/format.hpp"

using namespace std;

namespace cs
{
namespace core
{
namespace protocol
{

/*
 * Handlers -------------------------------------
 */

class MessageHandler_INITIAL: public MessageHandler
{
public:
    MessageHandler_INITIAL(State state, Protocol& protocol):
        MessageHandler{state, protocol}
    {
    }

    /**
     * Start on INITIAL
     */
    void visit(const msg::Start& msg) override
    try
    {
        share::Share& share = r_protocol.share(msg.m_share_id);

        r_protocol.m_peerinfo.m_peer = msg.m_peer;
        r_protocol.m_peerinfo.m_name = msg.m_name;
        // TODO: access
        r_protocol.m_peerinfo.m_features = msg.m_features;
        r_protocol.m_peerinfo.m_software = msg.m_software;

        const ServerInfo& si = r_protocol.r_serverinfo;
        r_protocol.send_msg(msg::Go(si.m_software, si.m_protocol, si.m_features, r_protocol.m_share, "", share.m_peer_id, si.m_name, utils::isotime(std::time(nullptr))));
        /***********/
        m_next_state = CONNECTED;
        /***********/
    }
    catch (...)
    {
        r_protocol.send_msg(msg::CannotStart());
        throw;
    }

    void visit(const msg::InternalSendStart& msg) override
    {
        share::Share& share = r_protocol.share(msg.m_share_id);
        // FIXME access, peer discovery
        const ServerInfo& si = r_protocol.r_serverinfo;
        r_protocol.send_msg(msg::Start(si.m_software, si.m_protocol, si.m_features, msg.m_share_id, "", share.m_peer_id, si.m_name, utils::isotime(std::time(nullptr))));
        /***********/
        m_next_state = WAIT4_GO;
        /***********/
    }

};


class MessageHandler_WAIT4_GO: public MessageHandler
{
public:
    MessageHandler_WAIT4_GO(State state, Protocol& protocol):
        MessageHandler{state, protocol}
    {
    }

    void visit(const msg::Go& msg) override
    {
        r_protocol.m_peerinfo.m_peer = msg.m_peer;
        r_protocol.m_peerinfo.m_name = msg.m_name;
        r_protocol.m_peerinfo.m_features = msg.m_features;
        r_protocol.m_peerinfo.m_software = msg.m_software;
        // FIXME access

        if (r_protocol.m_share != msg.m_share_id)
            throw std::runtime_error("Share id doesn't match in WAIT4_GO / msg::Go");

        /***********/
        m_next_state = CONNECTED;
        /***********/
    }
};



class MessageHandler_CONNECTED: public MessageHandler
{
public:
    MessageHandler_CONNECTED(State state, Protocol& protocol):
        MessageHandler{state, protocol}
    {
    }

    void visit(const msg::Get& msg) override
    {
        const bool ok = r_protocol.do_get(msg.m_checksum);
        if (ok)
            /***********/
            m_next_state = GET;
            /***********/
    }

    void visit(const msg::GetUpdates& msg) override
    {
        r_protocol.do_get_updates(msg.m_since);
    }

    void visit(const msg::Update& msg) override
    {
        r_protocol.do_update(msg.m_files);
        if (msg.m_partial)
            /***********/
            m_next_state = GET_UPDATES;
            /***********/
    }
};

class MessageHandler_GET_UPDATES: public MessageHandler
{
public:
    MessageHandler_GET_UPDATES(State state, Protocol& protocol):
        MessageHandler{state, protocol}
    {
    }
    // No messages are allowed while sending partial updates
    // After the transfer is finished we go back to CONNECTED in Protocol::handle_empty_output_buff

    void visit(const msg::Update& msg) override
    {
        r_protocol.do_update(msg.m_files);
        if (! msg.m_partial)
            /***********/
            m_next_state = CONNECTED;
            /***********/
    }
};




class MessageHandler_GET: public MessageHandler
{
public:
    MessageHandler_GET(State state, Protocol& protocol):
        MessageHandler{state, protocol}
    {
    }
    // No messages are allowed while sending data, TODO new mtype to abort?
    // After the transfer is finished we go back to CONNECTED in Protocol::handle_empty_output_buff

};






/*
 * Protocol -------------------------------------
 */





/**
 * ctor, sets message handlers.
 */
Protocol::Protocol(const ServerInfo& server_info, std::map<std::string, share::Share>& shares):
      r_serverinfo(server_info)
    , r_shares(shares)
    , m_peerinfo()
    , m_share()
    , m_state(State::INITIAL)
    , m_state_trans_table()
    , m_txfile_is()
    //, m_frozen_manifest()
    , m_rxfile_os()
    , m_coder()
    , m_handle_send_msg()
{
#define SET_HANDLER(state, type) m_state_trans_table[(state)] = make_unique<type>(m_state, *this);

    SET_HANDLER(INITIAL, MessageHandler_INITIAL);
    SET_HANDLER(WAIT4_GO, MessageHandler_WAIT4_GO);
    SET_HANDLER(CONNECTED, MessageHandler_CONNECTED);
    SET_HANDLER(GET, MessageHandler_GET);

#undef SET_HANDLER
}


void Protocol::send_msg(const msg::Message& m)
{
    assert(! m_txfile_is); // we are not sending data
    m_handle_send_msg(m_coder.encode_msg(m), m.m_payload);
}

void Protocol::send_file(const bfs::path& path)
{
    m_txfile_is = make_unique<bfs::ifstream>(path, ios_base::in | ios_base::binary);
    if (! *m_txfile_is)
    {
        m_txfile_is.reset();
        throw std::runtime_error(boost::str(boost::format("Protocol::send \"%1%\" error, couldn't open file") % path.string())); 
    }
}

void Protocol::recieve_file(const bfs::path& path)
{
    m_rxfile_os = make_unique<bfs::ofstream>(path, ios_base::out | ios_base::binary);
    m_rxfile_os->exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
    if (! *m_rxfile_os)
    {
        m_rxfile_os.reset();
        throw std::runtime_error(boost::str(boost::format("Protocol::send \"%1%\" error, couldn't open file") % path.string())); 
    }
}

/**
 * If we are writing a file, put next chunk in the output buffer
 */
void Protocol::handle_empty_output_buff()
{
    if (m_txfile_is)
    {
        // when the pointer is not null, a file transfer is in progress, send the next chunk
        std::string rbuff(s_txfile_block_sz, 0);
        m_txfile_is->read(&rbuff[0], rbuff.size());
        rbuff.resize(m_txfile_is->gcount());
        // send the buffer
        m_handle_send_payload_chunk(move(rbuff));
        if (! *m_txfile_is)
        {
            // EOF
            if (! rbuff.empty())
                // make sure to send the terminating 0 size chunk, per cs payload protocol
                m_handle_send_payload_chunk(string());
            m_txfile_is.reset();
            assert(m_state == GET);
            /*****************/
            m_state = CONNECTED;
            /*****************/
        }
    }
}


void Protocol::handle_msg(char const* msg_encoded, size_t msg_sz, char const* signature, size_t signature_sz, bool payload)
{
    auto msg = m_coder.decode_msg(payload, msg_encoded, msg_sz, signature, signature_sz);
    unique_ptr<MessageHandler>& handler = m_state_trans_table[m_state];
    assert(handler);
    msg->accept(*handler);
    /********* m_next_state *****/
    m_state = handler->next_state();
    /**************/
}

void Protocol::handle_payload(const char* data, size_t len)
{
    if (m_rxfile_os)
        m_rxfile_os->write(data, len);
    else
        throw std::runtime_error("handle_payload unexpected payload, a file transfer is not in progress");
}

void Protocol::handle_payload_end()
{
    m_rxfile_os.reset();
}


void Protocol::handle_update(const std::vector<msg::MFile>& files)
{
    // FIXME
}

bool Protocol::do_get(const std::string& checksum)
{
    // get list of files that match this checksum from the share
    const auto mfiles = share().get_mfiles_by_content(checksum);
    #if 0
    vector<string> paths;
    // some might have been changed now, send only paths that are up to date
    transform(mfiles.begin(), mfiles.end(), back_inserter(paths), [](const share::MFile& x) {
        return x.path;
    });
    #endif
    if (! mfiles.empty())
    {
        msg::FileData filedata(checksum);
        assert(filedata.m_payload);
        send_msg(filedata);
        send_file(share().fullpath(bfs::path(mfiles.front().path)));
        return true;
    }
    else
    {
        send_msg(msg::NoSuchFile(checksum));
        return false;
    }
}

void Protocol::do_get_updates(const std::map<std::string, u64>& since)
{
    auto& share = this->share(); 
    auto frozen_manifest = share.get_updates(m_peerinfo.m_name, since);
    msg::Update update(share.m_revision);
    for (const auto& mfile: *frozen_manifest)
        update.m_files.emplace_back(move(mfile.to_msg_mfile()));

    // FIXME, what if it's too large?
    update.m_partial = false;
    send_msg(update);
}

void Protocol::do_update(const std::vector<msg::MFile>& files)
{
    auto& share = this->share();
    for_each(files.begin(), files.end(), bind(&share::Share::remote_update, &share, placeholders::_1));
}

share::Share& Protocol::share(const std::string& share)
{
    if (! share.empty())
        m_share = share;
    auto shr_i = r_shares.find(m_share);
    if (shr_i != r_shares.end())
        return shr_i->second;
    throw ShareNotFoundError(boost::str(boost::format("Share %1% can't be found") % m_share));
}

void connect(ProtocolState& pstate, Protocol& protocol)
{
    // bind output handlers
    protocol.m_handle_send_msg = bind(&ProtocolState::send_msg, &pstate, placeholders::_1, placeholders::_2);
    protocol.m_handle_send_payload_chunk = bind(&ProtocolState::send_payload_chunk, &pstate, placeholders::_1);

    // bind input handlers
    pstate.m_handle_msg = bind(&Protocol::handle_msg, &protocol, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5);
    pstate.m_handle_empty_output_buff = bind(&Protocol::handle_empty_output_buff, &protocol);
    pstate.m_handle_payload = bind(&Protocol::handle_payload, &protocol, placeholders::_1, placeholders::_2);
    pstate.m_handle_payload_end = bind(&Protocol::handle_payload_end, &protocol);
}


} // end ns
} // end ns
} // end ns
