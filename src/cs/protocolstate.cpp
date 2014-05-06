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
#include <cstdlib>
#include <cassert>
#include "ibytestream.hpp"

using namespace std;

namespace cs
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
 *
 * 
 *  s[4 bytes size]:<50bytes>[4 bytes size]:<24 bytes signature>
 *  m[4 bytes size]:<50bytes>
 *  ![4 bytes size]:<50bytes>[4 bytes chunk size]:<chunk size payload bytes>
 *
 */
MsgRstate find_message(const std::string& buff)
{
    static const size_t prefix_sz = 6; // s[sz]:
    static const size_t sign_prefix_sz = 5;
    MsgRstate result;
    if (buff.size() < prefix_sz)
        return result;
    result.prefix = buff[0];
    if (result.prefix != 'm' && result.prefix != '$' && result.prefix != 's' && result.prefix != '!')
        return result.set_garbage();

    io::Ibytestream ibytestream(reinterpret_cast<u8 const*>(buff.data()), reinterpret_cast<u8 const*>(buff.data() + buff.size()));
    ibytestream.skip(1);
    result.encoded_sz = ibytestream.read<u32>();

    if (*ibytestream.m_next != ':' || result.encoded_sz > ProtocolState::s_msg_size_max)
        return result.set_garbage();

    result.encoded = reinterpret_cast<const char*>(ibytestream.skip(1));
    ibytestream.skip(result.encoded_sz);

    const size_t msg_have = buff.size() - prefix_sz;
    if (msg_have >= result.encoded_sz)
    {
        // process message
        if (has_signature(result.prefix))
        {
            if (buff.size() >= prefix_sz + result.encoded_sz + sign_prefix_sz)
            {
                ibytestream.skip(1 + result.encoded_sz);
                result.signature_sz = ibytestream.read<u32>();
                if (*ibytestream.m_next != ':')
                    return result.set_garbage();

                if (result.encoded_sz + result.signature_sz > ProtocolState::s_msg_size_max)
                    // we don't like this
                    return result.set_garbage();

                result.signature = reinterpret_cast<const char*>(ibytestream.skip(1));
                ibytestream.skip(result.signature_sz);
                result.found = true;
                result.end = reinterpret_cast<const char*>(ibytestream.m_next) - buff.data();
                return result;
            }
            else
                // keep reading
                return result;
        }
        else
        {
            result.found = true;
            result.end = reinterpret_cast<const char*>(ibytestream.m_next) - buff.data();
            return result;
        }
    }
    // keep reading
    return result;
}


PayLoadFound find_payload(const std::string& buff)
{
    PayLoadFound result;
    static const size_t payload_prefix_sz = 5;
    io::Ibytestream ibytestream(reinterpret_cast<u8 const*>(buff.data()), reinterpret_cast<u8 const*>(buff.data() + buff.size()));

    result.data_sz = ibytestream.read<u32>();
    if (*ibytestream.m_next != ':')
        return result.set_garbage();

    if (result.data_sz > ProtocolState::s_payload_chunk_size_max)
        return result.set_garbage();

    if (buff.size() >= result.data_sz + payload_prefix_sz)
        result.found = true;
    return result;
}

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
                    m_read_payload = mrs.payload();
                    m_handle_msg(mrs.encoded, mrs.encoded_sz, mrs.signature, mrs.signature_sz, mrs.payload());
                }
                catch(...)
                {
                    m_handle_error();
                }
            }
            if (mrs.garbage)
                m_handle_error();

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
                    m_handle_payload(&m_input_buff[PayLoadFound::prefix_sz], m_pl_found.data_sz);
                else
                {
                    // last chunk has 0 size
                    m_handle_payload_end();
                    m_read_payload = false;
                }
                trim_buff(m_input_buff, cbegin(m_input_buff) + m_pl_found.total_size());
                m_pl_found.reset();
            }
            else if (m_pl_found.garbage)
            {
                m_handle_error();
                // FIXME: review this
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

void ProtocolState::send_msg(const std::string&& msg_encoded, bool const payload)
{
    assert(m_payload_ended);
    m_last_has_payload = payload;
    const bool do_write = m_output_buff.empty() == true;
    m_output_buff.emplace_back(move(msg_encoded));
    if (do_write)
        write_next_buff();
}

void ProtocolState::send_payload_chunk(const std::string& chunk)
{
    assert(m_last_has_payload);
    const bool do_write = m_output_buff.empty() == true;
    m_payload_ended = chunk.empty();
    ostringstream os;
    os << chunk.size() << "\n";
    m_output_buff.emplace_back(os.str()); // size prefix
    m_output_buff.emplace_back(chunk); // payload chunk
    // writes are like a chain, only if it's empty we start a write, otherwise on_write_finished
    // triggers writting of the next buffer.
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
        m_handle_empty_output_buff();
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
