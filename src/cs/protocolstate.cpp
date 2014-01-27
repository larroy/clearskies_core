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

using namespace std;

namespace cs
{
namespace protocol
{



MsgFound find_message(const std::string& buff)
{
    MsgFound result;
    const size_t newline_pos = buff.find('\n');

    /// minimum message: {}\n
    if (newline_pos == string::npos)
        return result;

    result.end = buff.begin() + newline_pos;
    if (buff.size() < 3)
    {
        result.garbage = true;
        return result;
    }

    auto buff_i = buff.begin();
    if (*buff_i != '{')
    {
        if (*(buff_i+1) != '{')
        {
            result.garbage = true;
            return result;
        }
        result.prefix = *buff_i;
        ++buff_i;
    }
    result.json.assign(buff_i, result.end);
    result.found = true;
    return result;
}
   
void ProtocolState::input(const char* data, size_t len)
{
    m_input_buff.append(data, len);
}


} // end ns 
} // end ns 

