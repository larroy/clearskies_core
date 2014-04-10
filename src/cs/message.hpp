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
    GET_UPDATES,
    /// response to GET_UPDATES when there are no updates
    CURRENT,
    /// request to retrieve contents of a file
    GET,
    /// response with contents of a file
    FILE_DATA,
    /// notification of changed file
    UPDATE,
    /// notification of moved file
    MOVE,

    /// Not a message, Maximum value of the enum used to create arrays
    MAX,
};

std::string mtype_to_string(MType type);
MType mtype_from_string(const std::string& type);


enum class MAccess: unsigned
{
    UNKNOWN = 0,
    READ_ONLY,
    READ_WRITE,
};

std::string maccess_to_string(MAccess access);
MAccess maccess_from_string(const std::string& access);


struct MFile
{
    std::string m_path;
    double m_utime;
    long long m_size;
    std::vector<int> m_mtime;
    std::string m_mode;
    std::string m_sha256;
    bool m_deleted;
};


// forward declaration of message classes to avoid circular dependency below
class Unknown;
class InternalStart;
class Ping;
class Greeting;
class Start;
class CannotStart;
class StartTLS;
class Identity;
class Keys;
class KeysAcknowledgment;
class Manifest;
class GetUpdates;
class Current;
class Get;
class FileData;
class Update;
class Move;


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
    virtual void visit(const StartTLS&) = 0;
    virtual void visit(const Identity&) = 0;
    virtual void visit(const Keys&) = 0;
    virtual void visit(const KeysAcknowledgment&) = 0;
    virtual void visit(const Manifest&) = 0;
    virtual void visit(const GetUpdates&) = 0;
    virtual void visit(const Current&) = 0;
    virtual void visit(const Get&) = 0;
    virtual void visit(const FileData&) = 0;
    virtual void visit(const Update&) = 0;
    virtual void visit(const Move&) = 0;
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
    virtual void visit(StartTLS&) = 0;
    virtual void visit(Identity&) = 0;
    virtual void visit(Keys&) = 0;
    virtual void visit(KeysAcknowledgment&) = 0;
    virtual void visit(Manifest&) = 0;
    virtual void visit(GetUpdates&) = 0;
    virtual void visit(Current&) = 0;
    virtual void visit(Get&) = 0;
    virtual void visit(FileData&) = 0;
    virtual void visit(Update&) = 0;
    virtual void visit(Move&) = 0;
};


class Message
{
public:
    static size_t MAX_SIZE;

protected:
    /**
     * Message with specific type only to be used from derived classes
     */
    explicit Message(MType type):
          m_type(type)
        , m_payload(false)
        , m_signature()
    {
    }

public:
    virtual ~Message() = default;

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

template <typename CLASS, MType TYPE>
class MessageImpl: public Message
{
protected:
    typedef MessageImpl<CLASS, TYPE> THIS;

    MessageImpl():
        Message(TYPE)
    {
        static_assert(std::is_base_of<THIS, CLASS>::value, "CLASS must inherit from MessageImpl");
    }

public:
    virtual void accept(ConstMessageVisitor& v) const override
    {
        v.visit(*static_cast<const CLASS*>(this));
    }

    virtual void accept(MutatingMessageVisitor& v) override
    {
        v.visit(*static_cast<CLASS*>(this));
    }
};

/**
 * A message whose type we don't recognize
 */
class Unknown: public MessageImpl<Unknown, MType::UNKNOWN>
{
public:
    std::string m_content;
};

/**
 * Internal message to start the ClearSkiesProtocol and send a greeting
 */
class InternalStart: public MessageImpl<InternalStart, MType::INTERNAL_START>
{
};


class Ping: public MessageImpl<Ping, MType::PING>
{
public:
    u32 m_timeout = 60;
};


class Greeting: public MessageImpl<Greeting, MType::GREETING>
{
public:
    std::string m_software;
    std::vector<int> m_protocol;
    std::vector<std::string> m_features;
};


class Start: public MessageImpl<Start, MType::START>
{
public:
    Start(const std::string& software,
        const int protocol,
        const std::vector<std::string>& features,
        const std::string& id,
        const std::string& access,
        const std::string& peer
    ):
          m_software(software)
        , m_protocol(protocol)
        , m_share_id(id)
        , m_access(access)
        , m_peer(peer)
    {}

    Start():
          m_software()
        , m_protocol()
        , m_share_id()
        , m_access()
        , m_peer()
    {}

    bool operator==(const Start& o)
    {
        return std::tie(m_software, m_protocol, m_features, m_share_id, m_access, m_peer) ==
            std::tie(o.m_software, o.m_protocol, o.m_features, o.m_share_id, o.m_access, o.m_peer);
    }

    bool operator!=(const Start& o)
    {
        return ! (*this == o);
    }

    std::string m_software;
    int m_protocol = 0;
    std::vector<std::string> m_features;
    std::string m_share_id;
    std::string m_access;
    std::string m_peer;
};


class CannotStart: public MessageImpl<CannotStart, MType::CANNOT_START>
{
};


class StartTLS: public MessageImpl<StartTLS, MType::STARTTLS>
{
public:
    StartTLS():
          m_peer{}
        , m_access{MAccess::UNKNOWN}
    {}

    StartTLS(const std::string& peer, MAccess access):
          m_peer{peer}
        , m_access{access}
    {}

    bool operator==(const StartTLS& other)
    {
        return std::tie(m_peer, m_access) == std::tie(other.m_peer, other.m_access);
    }

    bool operator!=(const StartTLS& o)
    {
        return ! (*this == o);
    }


    std::string m_peer;
    MAccess m_access = MAccess::UNKNOWN;
};


class Identity: public MessageImpl<Identity, MType::IDENTITY>
{
public:
    Identity():
        m_name()
        , m_time()
    {}

    Identity(const std::string& name, const std::string& time):
        m_name(name)
        , m_time(time)
    {}

    std::string m_name;
    std::string m_time;
};


class Keys: public MessageImpl<Keys, MType::KEYS>
{
public:
    MAccess m_access = MAccess::UNKNOWN;
    std::string m_share_id;
    std::string m_ro_psk;
    std::string m_ro_rsa;
    std::string m_rw_public_rsa;
};


class KeysAcknowledgment: public MessageImpl<KeysAcknowledgment, MType::KEYS_ACKNOWLEDGMENT>
{
};


class Manifest: public MessageImpl<Manifest, MType::MANIFEST>
{
public:
    std::string m_peer;
    long long m_revision = 0;
    std::vector<MFile> m_files;
};


class GetUpdates: public MessageImpl<GetUpdates, MType::GET_UPDATES>
{
public:
    long long m_revision = 0;
};


class Current: public MessageImpl<Current, MType::CURRENT>
{
};

class Get: public MessageImpl<Get, MType::GET>
{
public:
    Get(const std::string& checksum):
        m_checksum(checksum)
    {}

    Get():
        m_checksum()
    {}

    std::string m_checksum;
};

class FileData: public MessageImpl<FileData, MType::FILE_DATA>
{
public:
    std::string m_path;
    std::vector<long long> m_range;
};

class Update: public MessageImpl<Update, MType::UPDATE>
{
public:
    long long m_revision = 0;
    MFile m_file;
};

class Move: public MessageImpl<Move, MType::MOVE>
{
public:
    long long m_revision = 0;
    std::string m_source;
    MFile m_destination;
};


} // end ns
} // end ns
