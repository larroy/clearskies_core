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

#include "int_types.h"
#include "message.hpp"
#include <stddef.h>


namespace cs
{
namespace protocol
{
/**
 * @brief Base protocol state class for all protocols
 * @author plarroy
 *
 * Input data is fed and when messages are assembled handle_message is called which implementes the
 * message dispatching logic
 */
class ProtocolState 
{
public:
    /**
     * feed input data, for example from socket IO
     * Once a full message is read, handle_message is called
     */
    ProtocolState():
        m_input_buff()
    {
        m_input_buff.reserve(4096);
    }

    //ProtocolState(const ProtocolState&) = delete;
    //ProtocolState& operator=(const ProtocolState&) = delete;

    virtual ~ProtocolState() = default;
    void input(const std::string& s)
    {
        input(s.c_str(), s.size()); 
    }
    void input(const char* data, size_t len);

    virtual void handle_message(const message::Message&) = 0;

private:
    /// FIXME: using a deque would be more efficient for appending data
    std::string m_input_buff;
    
};

struct MsgFound
{
    MsgFound():
        found(false),
        garbage(false),
        json(),
        prefix(),
        end()
    {}
    bool found;
    bool garbage;
    /// json part
    std::string json;
    /// prefix ! or $
    char prefix;
    /// position where json part ends
    std::string::const_iterator end;
};

MsgFound find_message(const std::string& buff);


} // end ns
} // end ns
