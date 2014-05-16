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

#include "../config.hpp"
#include "message.hpp"
#include "serverinfo.hpp"
#include "share.hpp"
#include "coder.hpp"
#include "../utils.hpp"
#include "../protocolstate.hpp"
#include <array>
#include <map>


/*
 * FIXME:
 *  + handle messages when payload is being sent, just cancellation should be allowed
 *  + handle security of send_file and recieve_file functions
 *
 */



/* Active is the peer that opens the connection
 *
 * Active    Start       Passive
 *        ---------->  
 *
 *           Go
 *        <----------  
 *
 *          GetUpdates(since: {A:1, B:2, C:3})
 *        ---------->  
 *
 *          Updates({
 *              revision,
 *              partial,
 *              values:
 *              [
 *                  {
 *                      checksum: "...",
 *                      paths: ["x/y", "z/k"],
 *                      last_changed_by,
 *                      last_changed_rev,size,...
 *                  },
 *                  ...
 *              ]
 *          )
 *        <--------
 *
 *         Get({checksum}) 
 *
 *        ---------->  
 *
 *         FileData(
 *              {
 *              }
 *         )
 *
 *        <--------
 *
 *
 *
 *        ....
 *
 *        Updates
 *        ---------->  
 */

namespace cs
{
namespace core
{
namespace protocol
{

enum State: unsigned
{
    INITIAL = 0,
    WAIT4_GO,
    WAIT4_IDENTITY,
    CONNECTED,
    GET,
    ////
    MAX,
};


DEFINE_RE_EXCEPTION(ProtocolError);
DEFINE_RE_EXCEPTION(ShareNotFoundError);

class Protocol;

/**
 * Base class for handling messages, throws if the message type is not expected in the current
 * protocol state.
 *
 * To handle a message, we have to know in which state we want to handle it. We install a handler
 * for this state, and in the visit function we perform desired actions and if applicable, we change
 * m_next_state variable so the Protocol class switches to the next state after handling
 * the message.
 *
 * A message recieved in a state in which is not expected triggers a ProtocolError exception, which
 * should result in the connection being closed.
 *
 *
 * 1. Install handler: SET_HANDLER(STATE, VISITOR_CLASS)
 * 2. Implement Handler::visit(const msg::Type&)
 *  2.1 change m_next_state to the desired next state
 *
 * FIXME: write tests for these cases on the server that does network IO.
 *
 */
class MessageHandler: public msg::ConstMessageVisitor
{
public:
    explicit MessageHandler(State state, Protocol& protocol):
          m_state(state)
        , m_next_state(state)
        , r_protocol(protocol)
    {}

    virtual State next_state()
    {
        return m_next_state;
    }

// FIXME: message type in string

    void visit(const msg::Unknown&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::InternalSendStart&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Ping&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Greeting&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Start&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::CannotStart&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Go&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Identity&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Keys&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::KeysAcknowledgment&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Manifest&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::GetUpdates&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Current&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Get&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::FileData&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::FileModified&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Update&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }
    void visit(const msg::Move&) override
    {
        throw ProtocolError(fs("Can't handle message type Unknown on state: " << static_cast<unsigned>(m_state)));
    }

    State m_state;
    State m_next_state;
    Protocol& r_protocol;
};

/**
 * state transition table, for each state, we have a handler for all the message types, which is a
 * message visitor. The default behaviour when not expecting a message in the given state is to
 * throw ProtocolError.
 */
typedef std::array<std::unique_ptr<MessageHandler>, State::MAX> state_trans_table_t;

/**
 * Implements the clearskies core protocol
 */
class Protocol
{
public:
    typedef std::function<void(const std::string&& msg_sig_encoded, bool payload)> handle_send_msg_t;
    typedef std::function<void(const std::string& chunk)> handle_send_payload_chunk_t;

    Protocol(const ServerInfo&, std::map<std::string, share::Share>& shares);

    void send_msg(const msg::Message& m);

    State state() const { return m_state; }
    void set_state(State state) { m_state = state; }

    /**
     * open the given file and set m_txfile_is so payload chunks are read and queued to be sent each time
     * handle_empty_output_buff is called when the output buffers are empty
     * 
     * Warning: Caller is responsible for the security of this function and permissions to access the given
     * path
     *
     *
     * @throws runtime_error when file can't be opened
     */
    void send_file(const bfs::path& path);

    /**
     * open the given file for writing so recieved chunks are written there on handle_payload
     *
     * Warning: Caller is responsible for the security of this function and permissions to access the given
     *
     * @throws runtime_error when file can't be opened
     */
    void recieve_file(const bfs::path& path);

    // callbacks for connecting to @sa cs::ProtocolState
    void handle_empty_output_buff();
    void handle_msg(char const* msg_encoded, size_t msg_sz, char const* signature, size_t signature_sz, bool payload);
    void handle_payload(const char* data, size_t len);
    void handle_payload_end();

    // message actions, note that the handlers / visitors logic control that these actions are triggered on the
    // appropiate states only

    /// action for MType::GET
    void do_get(const std::string& checksum);

    /**
     * @returns the current selected @param[in] share or throws ShareNotFoundError, this can happen if the
     * share was detached and can't be found anymore, in this case users of this class should close
     * the connection to clients.
     *
     * If no share is specified, it returns the current share
     */
    share::Share& share(const std::string& share = std::string());

    const ServerInfo& r_server_info;
    /// a reference to the shares
    std::map<std::string, share::Share>& r_shares;
    /// selected share name
    std::string m_share;
    /// peer id of the connected peer
    std::string m_peer;
    /// @sa msg::Start
    std::string m_access;
    std::vector<std::string> m_features;
    std::string m_software;

    /// current protocol state
    State m_state;

    /// table of message visitors given a state
    state_trans_table_t m_state_trans_table;
    /// the file being transmitted to the peer when set
    static const size_t s_txfile_block_sz = 65536;
    /// pointer to an open input stream for the file that is being sent if set
    std::unique_ptr<bfs::ifstream> m_txfile_is;
    /// pointer to an open output stream for the file that is being recieved if set
    std::unique_ptr<bfs::ofstream> m_rxfile_os;

    /// encodes messages into bytes
    msg::Coder m_coder;
    handle_send_msg_t m_handle_send_msg;
    handle_send_payload_chunk_t m_handle_send_payload_chunk;
};

void connect(ProtocolState&, Protocol&);

} // end ns
} // end ns
} // end ns
