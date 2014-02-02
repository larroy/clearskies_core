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
#include "utils.hpp"
#include <string>
#include <vector>

namespace cs
{
namespace message
{


enum class MType: unsigned
{
    UNKNOWN = 0,

    // Internal messages
    INTERNAL_START,

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

// forward declaration of message classes to avoid circular dependency below
class Unknown;
class InternalStart;
class Ping;
class Greeting;
class Start;
class CannotStart;


class ConstMessageVisitor
{
public:
    virtual ~ConstMessageVisitor() {};
    virtual void visit(const Unknown&) = 0;
    virtual void visit(const InternalStart&) = 0;
    virtual void visit(const Ping&) = 0;
    virtual void visit(const Greeting&) = 0;
    virtual void visit(const Start&) = 0;
    virtual void visit(const CannotStart&) = 0;
};


class MutatingMessageVisitor
{
public:
    virtual ~MutatingMessageVisitor() {};
    virtual void visit(Unknown&) = 0;
    virtual void visit(InternalStart&) = 0;
    virtual void visit(Ping&) = 0;
    virtual void visit(Greeting&) = 0;
    virtual void visit(Start&) = 0;
    virtual void visit(CannotStart&) = 0;
};


class Message
{
public:
    static size_t MAX_SIZE;

protected:
    /**
     * Message with specific type only to be used from derived clases
     */
    Message(MType type):
          m_type(type)
        , m_payload(false)
        , m_signature()
    {
    }

public:
    virtual ~Message() {};

    Message(const Message&) = default;
    Message(Message&&) = default;
    Message& operator=(const Message&) = default;
    Message& operator=(Message&&) = default;

    virtual void accept(ConstMessageVisitor& v) const = 0;
    virtual void accept(MutatingMessageVisitor& v) = 0;

    /// @return type of message
    MType type() const
    {
        return m_type;
    }

    bool payload() const
    {
        return m_payload;
    }

    bool signature() const
    {
        return ! m_signature.empty();
    }

    MType m_type;
    bool m_payload;
    std::string m_signature;
};


/**
 * A message whose type we don't recognize
 */
class Unknown: public Message
{
public:
    Unknown():
        Message(MType::UNKNOWN)
    {}

    virtual void accept(ConstMessageVisitor& v) const override { v.visit(*this); }
    virtual void accept(MutatingMessageVisitor& v) override { v.visit(*this); }

    std::string m_content;
};

/**
 * Internal message to start the ClearSkiesProtocol and send a greeting
 */
class InternalStart: public Message
{
public:
    InternalStart():
        Message(MType::INTERNAL_START)
    {}

    virtual void accept(ConstMessageVisitor& v) const override { v.visit(*this); }
    virtual void accept(MutatingMessageVisitor& v) override { v.visit(*this); }
};


class Ping: public Message
{
public:
    Ping():
          Message(MType::PING)
        , m_timeout(60)
    {
    }

    virtual void accept(ConstMessageVisitor& v) const override { v.visit(*this); }
    virtual void accept(MutatingMessageVisitor& v) override { v.visit(*this); }

    u32 m_timeout;
};


class Greeting: public Message
{
public:
    Greeting():
          Message(MType::GREETING)
        , m_software()
        , m_protocol()
        , m_features()
    {
    }

    virtual void accept(ConstMessageVisitor& v) const override { v.visit(*this); }
    virtual void accept(MutatingMessageVisitor& v) override { v.visit(*this); }


    std::string m_software;
    std::vector<int> m_protocol;
    std::vector<std::string> m_features;
};


class Start: public Message
{
public:
    Start():
          Message(MType::START)
        , m_software()
        , m_protocol()
        , m_features()
        , m_id()
        , m_access()
        , m_peer()
    {
    }

    virtual void accept(ConstMessageVisitor& v) const override { v.visit(*this); }
    virtual void accept(MutatingMessageVisitor& v) override { v.visit(*this); }


    std::string m_software;
    std::vector<int> m_protocol;
    std::vector<std::string> m_features;
    std::string m_id;
    std::string m_access;
    std::string m_peer;
};


class CannotStart: public Message
{
public:
    CannotStart():
          Message(MType::CANNOT_START)
    {
    }

    virtual void accept(ConstMessageVisitor& v) const override { v.visit(*this); }
    virtual void accept(MutatingMessageVisitor& v) override { v.visit(*this); }

};




} // end ns
} // end ns
