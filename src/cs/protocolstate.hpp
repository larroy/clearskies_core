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
#include <string>
#include <deque>
#include <functional>
#include <cassert>
#include <stddef.h>


namespace cs
{

inline bool has_signature(const char c)
{
    return c == '$' || c == 's';
}


inline bool has_payload(const char c)
{
    return c == '$' || c == '!';
}


struct MsgRstate
{
    MsgRstate():
          prefix()
        , found(false)
        , garbage(false)
        , encoded()
        , encoded_sz()
        , signature()
        , signature_sz()
        , enc_sig_sz()
    {}

    MsgRstate& set_garbage()
    {
        garbage = true;
        return *this;
    }

    bool payload() const
    {
        return has_payload(prefix);
    }

    bool has_signature() const
    {
        return signature_sz != 0;
    }

    /// prefix !: payload s: signed $: signed payload
    char prefix;
    bool found;
    bool garbage;
    const char* encoded;
    size_t encoded_sz;
    const char* signature;
    size_t signature_sz;
    /// pos where msg ends, data is processed and destroyed until this pos
    size_t enc_sig_sz;
};


/// @return where the message starts and its components in the input buffer
MsgRstate find_message(const std::string& buff);


struct PayLoadFound
{
    PayLoadFound():
        found()
        , garbage()
        , data_sz()
    {}

    void reset()
    {
        found = false;
        garbage = false;
        data_sz = 0;
    }

    PayLoadFound& set_garbage()
    {
        garbage = true;
        return *this;
    }

    bool error() const
    {
        return garbage;
    }

    explicit operator bool() const
    {
        return found;
    }

    size_t total_size() const
    {
        assert(found);
        return prefix_sz + data_sz;
    }

    bool found;
    bool garbage;
    size_t data_sz;
    static const size_t prefix_sz = 5;
};

/// @return info about a payload chunk on the input buffer
PayLoadFound find_payload(const std::string& buff);

/**
 * @brief Base protocol state class for all protocols
 * @author larroy
 *
 * Low level buffer IO handling
 *
 * Input data is fed and when messages are complete, handle_message is called which implementes the
 * message dispatching logic in derived classes
 */
class ProtocolState
{
public:
    /// type of callback for writing data
    typedef std::function<void(char const*, size_t)> do_write_t;

    /// called when a message is completely read on the input buffer
    typedef std::function<void(char const* msg_encoded, size_t msg_sz, char const* signature, size_t signature_sz, bool payload)> handle_msg_t;

    /// called by on_write_finished to signal that we are out of data (ex. send more manifest
    /// messages, or send the next chunk of payload)
    typedef std::function<void(void)> handle_empty_output_buff_t;

    /// called after a message with the payload flag was handled and payload was input
    typedef std::function<void(char const* data, size_t len)> handle_payload_t;

    /// called at the end of payload (record of size 0)
    typedef std::function<void()> handle_payload_end_t;

    /// when the protocol can't make sense of the data we should probably close the connection
    typedef std::function<void()> handle_error_t;


    static size_t s_msg_signature_max;
    static size_t s_msg_size_max;
    static size_t s_payload_chunk_size_max;
    /// initial size of the input buffer
    static size_t s_input_buff_size;
    ProtocolState():
          m_input_buff()
        , m_output_buff()
        , m_last_has_payload()
        , m_payload_ended(true)
        , m_read_payload(false)
        , m_pl_found()
        , m_do_write([](char const*, size_t) { assert(false); })
        , m_write_in_progress(false)
        , m_handle_empty_output_buff()
        , m_handle_msg()
        , m_handle_payload()
        , m_handle_payload_end()
        , m_handle_error([]() { assert(false);} )
    {
        m_input_buff.reserve(s_input_buff_size);
    }

#if 0
    ProtocolState(const ProtocolState&) = delete;
    ProtocolState& operator=(const ProtocolState&) = delete;
    ProtocolState(ProtocolState&&) = default;
    ProtocolState& operator=(ProtocolState&&) = default;
#endif

    void input(const std::string& s)
    {
        input(s.c_str(), s.size());
    }
    /**
     * feed input data, for example from socket IO
     * Once a full message is read, handle_message is called
     *
     * to be called by the event library on read
     */
    void input(const char* data, size_t len);

    void send_msg(const std::string&& msg_sig_encoded, bool payload);
    void send_payload_chunk(const std::string& chunk);

    void set_write_fun(do_write_t do_write)
    {
        m_do_write = do_write;
    }


    /// to be called by the event library on write when the last write finished
    void on_write_finished();

    /**
     * will write the next output buffer by calling the write function @sa m_do_write
     * @post m_write_in_progress will be true
     */
    void write_next_buff();


private:
    /// internal input buffer accumulating data until it can be processed
    std::string m_input_buff;
    /// queue of buffers to write, we write from front to back, new appended to back, when wrote,
    /// removed from front.
    std::deque<std::string> m_output_buff;

    bool m_last_has_payload;
    bool m_payload_ended;

    /// true if we are reading a payload section, false if we are reading or expecting a message
    bool m_read_payload;
    PayLoadFound m_pl_found;

public:
    /// callback used to write data
    do_write_t m_do_write;
    bool m_write_in_progress;

    handle_empty_output_buff_t m_handle_empty_output_buff;
    handle_msg_t m_handle_msg;
    handle_payload_t m_handle_payload;
    handle_payload_end_t m_handle_payload_end;
    handle_error_t m_handle_error;
};
} // end ns
