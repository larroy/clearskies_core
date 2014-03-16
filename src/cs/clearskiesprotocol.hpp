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
#pragma once

#include "config.hpp"
#include "protocolstate.hpp"
#include "serverinfo.hpp"
#include "share.hpp"
#include <array>
#include <map>

namespace cs
{
namespace protocol
{

enum State: unsigned
{
    // initial state, we can start and send a greeting by sending a message to ourselves.
    INITIAL = 0,
    WAIT4_CLIENT_IDENTITY,
    CONNECTED,


    MAX,
};

class ProtocolError: public std::runtime_error
{
public:
    explicit ProtocolError(const std::string& what):
        std::runtime_error(what)
    {}
};

class ClearSkiesProtocol;

/**
 * Base class for handling messages, throws if the message type is not expected in the current
 * protocol state.
 *
 * To handle a message, we have to know in which state we want to handle it. We install a handler
 * for this state, and in the visit function we perform desired actions and if applicable, we change
 * m_next_state variable so the ClearSkiesProtocol class switches to the next state after handling
 * the message.
 *
 *
 * 1. Install handler
 *   m_state_trans_table[State::INITIAL] = make_unique<MessageHandler_INITIAL>(m_state, *this);
 * 2. Implement Handler::visit(const message::Type&)
 *  2.1 change m_next_state to the desired next state
 *
 * If an unexpected message in some state is recieved, and exception is thrown which will cause the
 * connection to be closed immediately. FIXME: write tests for these cases on the server that does
 * network IO.
 *
 */
class MessageHandler: public message::ConstMessageVisitor
{
public:
    explicit MessageHandler(State state, ClearSkiesProtocol& protocol):
          m_state(state)
        , m_next_state(state)
        , r_protocol(protocol)
    {}

    virtual State next_state()
    {
        return m_next_state;
    }

// FIXME: message type in string

    void visit(const message::Unknown&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::InternalStart&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Ping&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Greeting&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Start&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::CannotStart&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::StartTLS&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Identity&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Keys&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::KeysAcknowledgment&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Manifest&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::GetUpdates&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::ManifestCurrent&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Get&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::FileData&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Update&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const message::Move&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }

    State m_state;
    State m_next_state;
    ClearSkiesProtocol& r_protocol;
};

/**
 * state transition table, for each state, we have a handler for all the message types, which is a
 * message visitor. The default behaviour when not expecting a message in the given state is to
 * throw ProtocolError.
 */
typedef std::array<std::unique_ptr<MessageHandler>, State::MAX> state_trans_table_t;

/**
 * This class specializes ProtocolState to implement the clearskies protocol on the message level
 */
class ClearSkiesProtocol: public ProtocolState
{
public:
    ClearSkiesProtocol(const ServerInfo&, const std::map<std::string, share::Share>& shares);
    State state() const { return m_state; }
    void set_state(State state) { m_state = state; }

    // overrides from ProtocolState
    void handle_message(std::unique_ptr<message::Message>) override;
    void handle_payload(const char* data, size_t len) override;
    void handle_payload_end() override;
    void handle_msg_garbage(const std::string& buff) override;
    void handle_pl_garbage(const std::string& buff) override;

    const ServerInfo& r_server_info;
    const std::map<std::string, share::Share>& r_shares;
    State m_state;

    state_trans_table_t m_state_trans_table;
};


} // end ns
} // end ns
