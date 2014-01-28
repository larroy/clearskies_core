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
#include "jsoncons/json.hpp"
#include "int_types.h"
#include "utils.hpp"

namespace cs
{
namespace message
{


enum class MType: unsigned
{
    UNKNOWN = 0,

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

std::string mtype_to_string(MType type);
MType mtype_from_string(const std::string& type);

class MessageError: public std::runtime_error
{
public:
    MessageError(const std::string& x):
        std::runtime_error(x)
    {}
};

class Message
{
public:
    Message(const Message&) = default;
    Message(Message&&) = default;
    Message& operator=(const Message&) = default;
    Message& operator=(Message&&) = default;

    /**
     * Construct a message from a json string
     * @throws MessageError on failing to parse message
     * if the type field is not known or pressent type() is UNKNOWN
     */
    Message(const std::string& json, bool payload = false, const std::string& signature = std::string());
protected:
    /**
     * Message with specific type only to be used from derived clases
     */
    Message(MType type):
          m_type(type)
        , m_has_payload(false)
        , m_json()
        , m_signature()
    {
    }

public:

    virtual ~Message() {}


    template<class ConcreteMessageType>

    /**
     * @throws MessageError if the message is invalid
     */
    ConcreteMessageType refine()
    {
        return ConcreteMessageType(m_json);
    }

    bool is_known() const
    {
        return m_type != MType::UNKNOWN;
    }

    /// @return type of message
    MType type() const
    {
        return m_type;
    }

    /// @return true if the message has the structure that we expect
    virtual bool valid() const
    {
        return m_type != MType::UNKNOWN;
    }

    MType m_type;
    bool m_has_payload;
    jsoncons::json m_json;
    std::string m_signature;
};

class Ping: public Message
{
public:
    Ping():
          Message(MType::PING)
        , m_timeout()
    {
    }

    /**
     * @throws MessageError if the message is invalid
     */
    Ping(const jsoncons::json& json):
          Message(MType::PING)
        , m_timeout()
    {
        // get timeout

    }

    bool valid() const override
    {
        if (m_type == MType::PING)
            return true;
        return false;
    }
    u32 m_timeout;
};


} // end ns
} // end ns
