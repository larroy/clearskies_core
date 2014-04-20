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
#include "clearskiesprotocol.hpp"
#include <cassert>
#include "boost/format.hpp"

using namespace std;

namespace cs
{
namespace protocol
{


/*
 * Handlers -------------------------------------
 */

class MessageHandler_INITIAL: public MessageHandler
{
public:
    MessageHandler_INITIAL(State state, ClearSkiesProtocol& protocol):
        MessageHandler{state, protocol}
    {
    }

    /**
     * Start on INITIAL
     */
    void visit(const message::Start& msg) override
    try
    {
        r_protocol.m_share = msg.m_share_id;
        share::Share& share = r_protocol.share();
        r_protocol.send_message(message::StartTLS(share.m_peer_id, message::MAccess::READ_WRITE));
        r_protocol.send_message(message::Identity(r_protocol.r_server_info.m_name, utils::isotime(std::time(nullptr))));
        m_next_state = WAIT4_CLIENT_IDENTITY;
    }
    catch (...)
    {
        r_protocol.send_message(message::CannotStart());
        throw;
    }
};


class MessageHandler_WAIT4_CLIENT_IDENTITY: public MessageHandler
{
public:
    MessageHandler_WAIT4_CLIENT_IDENTITY(State state, ClearSkiesProtocol& protocol):
        MessageHandler{state, protocol}
    {
    }

    void visit(const message::Identity& msg) override
    {
        m_next_state = CONNECTED;
    }
};


class MessageHandler_CONNECTED: public MessageHandler
{
public:
    MessageHandler_CONNECTED(State state, ClearSkiesProtocol& protocol):
        MessageHandler{state, protocol}
    {
    }

    void visit(const message::Identity& msg) override
    {
    }

    void visit(const message::Get& msg) override
    {
        r_protocol.do_get(msg.m_checksum);
        m_next_state = GET;
    }
};






/*
 * ClearSkiesProtocol -------------------------------------
 */





/**
 * ctor, sets message handlers.
 */
ClearSkiesProtocol::ClearSkiesProtocol(const ServerInfo& server_info, std::map<std::string, share::Share>& shares):
    ProtocolState()
    , r_server_info(server_info)
    , r_shares(shares)
    , m_share()
    , m_state(State::INITIAL)
{
#define SET_HANDLER(state, type) m_state_trans_table[(state)] = make_unique<type>(m_state, *this);

    SET_HANDLER(INITIAL, MessageHandler_INITIAL);
    SET_HANDLER(WAIT4_CLIENT_IDENTITY, MessageHandler_WAIT4_CLIENT_IDENTITY);
    SET_HANDLER(CONNECTED, MessageHandler_CONNECTED);

#undef SET_HANDLER
}

void ClearSkiesProtocol::send_file(const bfs::path& path)
{
    m_txfile_is = make_unique<bfs::ifstream>(path, ios_base::in | ios_base::binary);
    if (! *m_txfile_is)
    {
        m_txfile_is.reset();
        throw std::runtime_error(boost::str(boost::format("ClearSkiesProtocol::send \"%1%\" error, couldn't open file") % path.string())); 
    }
}

void ClearSkiesProtocol::recieve_file(const bfs::path& path)
{
    // FIXME: set ios exceptions
    m_rxfile_os = make_unique<bfs::ofstream>(path, ios_base::out | ios_base::binary);
    if (! *m_rxfile_os)
    {
        m_rxfile_os.reset();
        throw std::runtime_error(boost::str(boost::format("ClearSkiesProtocol::send \"%1%\" error, couldn't open file") % path.string())); 
    }
}

/**
 * If we are writing a file, put next chunk in the output buffer
 */
void ClearSkiesProtocol::handle_empty_output_buff()
{
    if (m_txfile_is)
    {
        // when the pointer is not null, a file transfer is in progress, send the next chunk
        std::string rbuff(s_txfile_block_sz, 0);
        m_txfile_is->read(&rbuff[0], rbuff.size());
        rbuff.resize(m_txfile_is->gcount());
        // send the buffer
        send_payload_chunk(move(rbuff));
        if (! *m_txfile_is)
        {
            // EOF
            if (! rbuff.empty())
                // make sure to send the terminating 0 size chunk, per cs payload protocol
                send_payload_chunk(string());
            m_txfile_is.reset();
        }
    }
}


void ClearSkiesProtocol::handle_message(std::unique_ptr<message::Message> msg)
{
    unique_ptr<MessageHandler>& handler = m_state_trans_table[m_state];
    assert(handler);
    msg->accept(*handler);
    m_state = handler->next_state();
}

void ClearSkiesProtocol::handle_payload(const char* data, size_t len)
{
    if (m_rxfile_os)
        m_rxfile_os->write(data, len);
    else
        // FIXME handle unexpected payload
        assert(0);
}

void ClearSkiesProtocol::handle_payload_end()
{
    m_rxfile_os.reset();
}

void ClearSkiesProtocol::handle_msg_garbage(const std::string& buff)
{
    assert(false);
}

void ClearSkiesProtocol::handle_pl_garbage(const std::string& buff)
{
    assert(false);
}

void ClearSkiesProtocol::do_get(const std::string& checksum)
{
    // get list of files that match this checksum from the share
    const auto mfiles = share().get_mfiles_by_content(checksum);
    vector<string> paths;
    // some might have been changed now, send only paths that are up to date
    transform(mfiles.begin(), mfiles.end(), back_inserter(paths), [](const share::MFile& x) {
        return x.path;
    });
    if (! paths.empty())
    {
        message::FileData filedata(move(paths));
        assert(filedata.m_payload);
        send_message(filedata);
        send_file(share().fullpath(bfs::path(filedata.m_paths.front())));
    }
    else
    {
        // FIXME!
    }
}

share::Share& ClearSkiesProtocol::share()
{
    auto shr_i = r_shares.find(m_share);
    if (shr_i != r_shares.end())
        return shr_i->second;
    throw ShareNotFoundError(boost::str(boost::format("Share %1% can't be found") % m_share));
}


} // end ns
} // end ns
