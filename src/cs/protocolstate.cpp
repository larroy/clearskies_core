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
#include "protocolstate.hpp"
#include "message.hpp"
#include <cstdlib>
#include <cassert>

using namespace std;

namespace cs
{
namespace protocol
{

namespace
{

pair<bool, size_t> to_base10(const char* b, const char * const e)
{
    pair<bool, size_t> res;
    assert(b <= e);
    size_t nchars = 0;
    while (b != e)
    {
    res.second *= 10;
        if (*b >= '0' && *b <= '9' && nchars < 20)
            res.second += *b - '0';
        else
        {
            res.first = false;
            return res;
        }
        ++b;
        ++nchars;
    }
    res.first = true;
    return res;
}

/// shift the unprocessed data in the input buffer left
void trim_buff(std::string& buf, std::string::const_iterator from)
{
    assert(from >= cbegin(buf));
    assert(from <= cend(buf));
    if (from == cbegin(buf))
        return;
    copy(from, cend(buf), begin(buf));
    buf.resize(distance(from, cend(buf)));
}

} // end anon ns


/**
 * m50:<49 bytes>\n
 *     ^
 *     |
 * preamble_end
 */
MsgRstate find_message(const std::string& buff)
{
    MsgRstate result;
    const size_t colon_pos = buff.find(':');
    if (colon_pos == string::npos)
    {
        if (buff.size() > ProtocolState::s_msg_preamble_max)
            return result.set_garbage();
        else
            // keep getting data
            return result;
    }
    assert(colon_pos != string::npos);
    if (colon_pos > ProtocolState::s_msg_preamble_max)
        return result.set_garbage();
    assert(! buff.empty());
    result.prefix = buff[0];
    auto b10 = to_base10(&buff[1], &buff[colon_pos]);
    if (! b10.first)
        return result.set_garbage();
    result.msg_len = b10.second;
    const size_t encoded_start = colon_pos + 1;
    const size_t encoded_end = encoded_start + result.msg_len; // \n position
    const size_t msg_have = buff.size() - encoded_start;
    size_t msg_plus_sig_end = encoded_end + 1;
    if (msg_have >= result.msg_len + 1)
    {
        // process message
        if (has_signature(result.prefix))
        {
            const size_t newline_pos = buff.find('\n', msg_plus_sig_end);
            if (newline_pos == string::npos)
            {
                assert(encoded_end <= buff.size());
                size_t signature_have = buff.size() - (encoded_end + 1);
                if (signature_have > ProtocolState::s_msg_signature_max)
                    return result.set_garbage();
                else
                    // keep reading
                    return result;
            }
            assert(newline_pos != string::npos);
            if (newline_pos > ProtocolState::s_msg_signature_max)
                return result.set_garbage();
            const size_t sig_start = msg_plus_sig_end;
            const size_t sig_end = newline_pos;
            msg_plus_sig_end = sig_end + 1;
            assert(sig_start <= sig_end);
            result.signature = &buff[sig_start];
            result.signature_sz = sig_end - sig_start;
        }
        assert(encoded_start <= encoded_end);
        result.encoded = &buff[encoded_start];
        result.encoded_sz = result.msg_len;
        result.found = true;
        result.end = msg_plus_sig_end;
        return result;
    }
    // keep reading
    return result;
}


PayLoadFound find_payload(const std::string& buff)
{
    PayLoadFound result;
    const size_t newline_pos = buff.find('\n');
    if (newline_pos == string::npos) // 9 comes from 16 MB limit in ascii + \n
    {
        if (buff.size() > 9)
        {
            // ignore all the garbage we recieved
            result.size_plus_newline_sz = buff.size();
            return result.set_garbage();
        }
        else
            // wait for \n
            return result;
    }
    assert(newline_pos != string::npos);
    result.size_plus_newline_sz = newline_pos + 1;
    if (result.size_plus_newline_sz > 21)
        // got too much stuff before \n
        return result.set_garbage();

    auto b10 = to_base10(&buff[0], &buff[newline_pos]);
    if (! b10.first)
        // something not numeric in base10
        return result.set_garbage();
    result.data_sz = b10.second;
    if (result.data_sz > ProtocolState::s_payload_chunk_size_max)
    {
        // ignore a chunk which is too big
        result.data_sz = 0;
        return result.set_garbage();
    }
    result.found = true;
    return result;
}

size_t ProtocolState::s_msg_preamble_max = 22; // m[20 digits]:
size_t ProtocolState::s_msg_signature_max = 512;
size_t ProtocolState::s_msg_size_max = 16777216;
size_t ProtocolState::s_payload_chunk_size_max = 16777216;
size_t ProtocolState::s_input_buff_size = 4096;

/**
 * Check if we have a full message then decode it and handle, otherwise wait for more data, same for
 * payload.
 */
void ProtocolState::input(const char* data, size_t len)
{
    m_input_buff.append(data, len);
    while (true)
    {
        if (! m_read_payload)
        {
            MsgRstate mrs = find_message(m_input_buff);
            if (mrs.found)
            {
                try
                {
                    unique_ptr<message::Message> msg = m_msg_coder.decode_msg(mrs.payload(), mrs.encoded, mrs.encoded_sz, mrs.signature, mrs.signature_sz);
                    if (mrs.payload())
                        m_read_payload = true;
                    handle_message(move(msg));
                }
                catch(const message::CoderError& e)
                {
                    handle_msg_garbage(fs("message::CoderError exception: " <<  e.what()));
                }
            }
            if (mrs.garbage)
                handle_msg_garbage(m_input_buff);

            trim_buff(m_input_buff, cbegin(m_input_buff) + mrs.end);
            if (mrs.end == 0)
                // no data was consumed, stop processing
                break;
        }
        else
        {
            if (! m_pl_found)
                // we are not waiting for finishing a chunk
                // read the size\n field
                m_pl_found = find_payload(m_input_buff);

            // we either found payload or not enough bytes to get the size
            if (m_pl_found && (m_input_buff.size() >= m_pl_found.total_size()))
            {
                if (m_pl_found.data_sz != 0)
                    handle_payload(&m_input_buff[m_pl_found.size_plus_newline_sz], m_pl_found.data_sz);
                else
                {
                    // last chunk has 0 size
                    handle_payload_end();
                    m_read_payload = false;
                }
                trim_buff(m_input_buff, cbegin(m_input_buff) + m_pl_found.total_size());
                m_pl_found.reset();
            }
            else if (m_pl_found.garbage)
            {
                handle_pl_garbage(m_input_buff);
                trim_buff(m_input_buff, cbegin(m_input_buff) + m_pl_found.total_size());
                m_read_payload = false;
                m_pl_found.reset();
            }
            else
                // we need more payload data or even for the size
                break;
        }
    }
}

void ProtocolState::send_message(const message::Message& m)
{
    assert(m_payload_ended);
    m_last_has_payload = m.payload();
    const bool do_write = m_output_buff.empty() == true;
    m_output_buff.emplace_back(m_msg_coder.encode_msg(m));
    if (do_write)
        write_next_buff();
}

void ProtocolState::send_payload_chunk(std::string&& chunk)
{
    assert(m_last_has_payload);
    const bool do_write = m_output_buff.empty() == true;
    m_payload_ended = chunk.empty();
    m_output_buff.emplace_back(move(chunk));
    if (do_write)
        write_next_buff();
}

void ProtocolState::on_write_finished()
{
    m_write_in_progress = false;
    assert(! m_output_buff.empty());
    m_output_buff.pop_front();
    if (! m_output_buff.empty())
        write_next_buff();
    else
        handle_empty_output_buff();
}

void ProtocolState::write_next_buff()
{
    assert(! m_write_in_progress);
    assert(! m_output_buff.empty());
    const string& buf = m_output_buff.front();
    m_do_write(buf.c_str(), buf.size());
    m_write_in_progress = true;
}

} // end ns
} // end ns
