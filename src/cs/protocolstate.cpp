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

using namespace std;

namespace cs
{
namespace protocol
{

namespace
{

inline bool has_signature(const char c)
{
    return c == '$' || c == '&';
}


inline bool has_payload(const char c)
{
    return c == '!' || c == '&';
}

}
MsgFound find_message(const std::string& buff)
{
    MsgFound result;
    result.end = buff.begin();
    const size_t newline1_pos = buff.find('\n');

    /// minimum message: {}\n
    if (newline1_pos == string::npos)
        return result;

    auto json_end = buff.begin() + newline1_pos;
    if (buff.size() < 3)
    {
        result.garbage = true;
        result.end = json_end + 1;
        return result;
    }

    auto buff_i = buff.begin();
    auto signature_end = buff.begin();
    auto signature_begin = buff.begin();
    if (*buff_i != '{')
    {
        if (*(buff_i + 1) != '{')
        {
            result.garbage = true;
            result.end = json_end + 1;
            return result;
        }
        result.prefix = *buff_i++;
        if (has_signature(result.prefix))
        {
            if (buff.size() > newline1_pos + 1)
            {
                const size_t newline2_pos = buff.find('\n', newline1_pos + 1);
                if (newline2_pos == string::npos)
                    return result;
                signature_begin = buff.begin() + newline1_pos + 1;
                signature_end = buff.begin() + newline2_pos;
                result.end = signature_end + 1;
            }
            else
                // accumulate more until we get also the signature
                return result;
        }
        else
            result.end = json_end + 1;
    }
    else
        result.end = json_end + 1;
    result.json.assign(buff_i, json_end);
    result.signature.assign(signature_begin, signature_end);
    result.found = true;
    return result;
}

void ProtocolState::input(const char* data, size_t len)
{
    m_input_buff.append(data, len);
    MsgFound found = find_message(m_input_buff);
    if (found.found)
    {
        message::Message msg(found.json, has_payload(found.prefix), found.signature);
        handle_message(msg);
    }
    if (found.end != cbegin(m_input_buff))
        m_input_buff.assign(found.end, cend(m_input_buff));
}


} // end ns
} // end ns

