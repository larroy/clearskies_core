/*
 *  This file is part of clearskies_core.

 *  clearskies_core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  clearskies_core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with clearskies_core.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

namespace cs
{
namespace message
{


enum class MType
{
    INVALID = 0,

    EMPTY,

    PING,
    GREETING,
    START,
    CANNOT_START,
    STARTTLS,
    IDENTITY,
    /// key content
    KEYS,
    /// response to keys
    KEYS_ACKNOWLEDGMENT,
    /// file listing
    MANIFEST,
    /// request for manifest
    GET_MANIFEST,
    /// response to GET_MANIFEST when revision matches
    MANIFEST_CURRENT,
    /// request to retrieve contents of a file
    GET,
    /// response with contents of a file
    FILE_DATA,
    /// notification of changed file
    UPDATE,
    /// notification of moved file
    MOVE,

    MAX,
};

class Message
{
public:
    Message(const Message&) = default;
    Message(Message&&) = default;
    Message& operator=(const Message&) & = default;
    Message& operator=(Message&&) & = default;

    virtual ~Message() {}

    
    template<class ConcreteMessageType>
    ConcreteMessageType refine()
    {
        return ConcreteMessageType::from_json(json); 
    }

    /// @return type of message
    virtual MType type() = 0;

    /// @return true if the message has the structure that we expect
    virtual bool valid() = 0;
    
    /// ! prefix indicates payload
    bool m_payload;

    /// $ prefix indicates signed message
    bool m_signed;
    jsoncons::json m_data;
};

class Ping: public Message
{
public:
    Ping(): 
        m_timeout()
    {}

    MType type() override;
    bool valid() override;
    u32 m_timeout;
};


} // end ns
} // end ns
