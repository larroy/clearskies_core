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

#include "protocolstate.hpp"
#include <array>

namespace cs
{
namespace protocol
{

enum class State: unsigned
{
    // initial state, we can start and send a greeting by sending a message to ourselves.
    INITIAL = 0,

    START_WAIT, // greeting was sent, waiting for start

    MAX,
};



// state transition function
typedef std::function<void(const message::Message&)> state_trans_fn_t;

/**
 * state transition matrix, message_type x protocol_state
 * each cell has a function pointer to execute when a message is recieved on the given state
 *
 */
#define SC(X) static_cast<size_t>(X)
typedef std::array<std::array<state_trans_fn_t, SC(State::MAX)>, SC(message::MType::MAX)> state_trans_table_t;
#undef SC

/**
 * This class specializes ProtocolState to implement the clearskies protocol on the message level
 */
class ClearSkiesProtocol: public ProtocolState
{
public:
    ClearSkiesProtocol(do_write_t do_write = [](const char*, size_t){}):
          ProtocolState(do_write)
        , m_state(State::INITIAL)
    {}

    State state() const { return m_state; }
    void set_state(State state) { m_state = state; }

    // overrides from ProtocolState
    void handle_message(std::unique_ptr<message::Message>) override;
    void handle_payload(const char* data, size_t len) override;
    void handle_payload_end() override;
    void handle_msg_garbage(const std::string& buff) override;
    void handle_pl_garbage(const std::string& buff) override;

    // state transition functions
    // trans_<State>_<Message type>
    void trans_INITIAL_INTERNAL_START(const message::Message&);

    State m_state;

    state_trans_table_t m_state_trans_table;
};


} // end ns
} // end ns
