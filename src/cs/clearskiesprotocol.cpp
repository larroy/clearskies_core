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

using namespace std;

namespace cs
{
namespace protocol
{


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
    {

        auto& shares = r_protocol.r_shares;
        auto shr_i = shares.find(msg.m_share_id);
        if (shr_i == shares.end())
        {
            r_protocol.send_message(message::CannotStart());
            return;
        }

        r_protocol.send_message(message::StartTLS(shr_i->second.m_peer_id, message::MAccess::READ_WRITE));
        r_protocol.send_message(message::Identity(r_protocol.r_server_info.m_name, utils::isotime(std::time(nullptr))));
    }
};


ClearSkiesProtocol::ClearSkiesProtocol(const ServerInfo& server_info, const std::map<std::string, share::Share>& shares):
    ProtocolState()
    , r_server_info(server_info)
    , r_shares(shares) // why using {} makes gcc compilation fail?
    , m_state{State::INITIAL}
{
    m_state_trans_table[State::INITIAL] = make_unique<MessageHandler_INITIAL>(m_state, *this);
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
}

void ClearSkiesProtocol::handle_payload_end()
{
}

void ClearSkiesProtocol::handle_msg_garbage(const std::string& buff)
{
    assert(false);
}

void ClearSkiesProtocol::handle_pl_garbage(const std::string& buff)
{
    assert(false);
}



} // end ns
} // end ns
