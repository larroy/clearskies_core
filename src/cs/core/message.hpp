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
#include "../int_types.h"
#include "../utils.hpp"
#include <string>
#include <vector>
#include <map>

namespace cs
{
namespace core
{
namespace msg
{

enum class MType: unsigned
{
    UNKNOWN = 0,

    // Internal messages
    INTERNAL_SEND_START,

    PING,
    START,
    GO,
    CANNOT_START,
    GET_UPDATES,
    /// response to GET_UPDATES when there are no updates
    GET,
    /// response with contents of a file
    FILE_DATA,
    NO_SUCH_FILE,
    UPDATE,

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
    MFile():
        checksum()
        , path()
        , last_changed_by()
        , mtime()
        , size()
        , deleted()
        , mode()
    {}

    MFile(const std::string& checksum,
        const std::string& path,
        const std::string& last_changed_by,
        const u64 last_changed_rev,
        const std::string& mtime,
        const u64 size,
        const bool deleted,
        const u16 mode)
    :
        checksum(checksum)
        , path(path)
        , last_changed_by(last_changed_by)
        , last_changed_rev(last_changed_rev)
        , mtime(mtime)
        , size(size)
        , deleted(deleted)
        , mode(mode)
    {}

    std::string checksum;
    std::string path;
    std::string last_changed_by;
    u64 last_changed_rev;
    std::string mtime;
    u64 size;
    bool deleted;
    u16 mode;
};


// forward declaration of message classes to avoid circular dependency below
class Unknown;
class InternalSendStart;
class Ping;
class Start;
class Go;
class CannotStart;
class GetUpdates;
class Get;
class FileData;
class NoSuchFile;
class Update;


class ConstMessageVisitor
{
public:
    virtual ~ConstMessageVisitor() {};
    virtual void visit(const Unknown&) = 0;
    virtual void visit(const InternalSendStart&) = 0;
    virtual void visit(const Ping&) = 0;
    virtual void visit(const Start&) = 0;
    virtual void visit(const Go&) = 0;
    virtual void visit(const CannotStart&) = 0;
    virtual void visit(const GetUpdates&) = 0;
    virtual void visit(const Get&) = 0;
    virtual void visit(const FileData&) = 0;
    virtual void visit(const NoSuchFile&) = 0;
    virtual void visit(const Update&) = 0;
};


class MutatingMessageVisitor
{
public:
    virtual ~MutatingMessageVisitor() {};
    virtual void visit(Unknown&) = 0;
    virtual void visit(InternalSendStart&) = 0;
    virtual void visit(Ping&) = 0;
    virtual void visit(Start&) = 0;
    virtual void visit(Go&) = 0;
    virtual void visit(CannotStart&) = 0;
    virtual void visit(GetUpdates&) = 0;
    virtual void visit(Get&) = 0;
    virtual void visit(FileData&) = 0;
    virtual void visit(NoSuchFile&) = 0;
    virtual void visit(Update&) = 0;
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
class InternalSendStart: public MessageImpl<InternalSendStart, MType::INTERNAL_SEND_START>
{
public:
    /// the share id of the initiated connection
    std::string m_share_id;
};


class Ping: public MessageImpl<Ping, MType::PING>
{
public:
    u32 m_timeout = 60;
};


class Start: public MessageImpl<Start, MType::START>
{
public:
    Start(
        const std::string& software,
        const int protocol,
        const std::vector<std::string>& features,
        const std::string& id,
        const std::string& access,
        const std::string& peer,
        const std::string& name,
        const std::string& time 
    ):
          m_software(software)
        , m_protocol(protocol)
        , m_features(features)
        , m_share_id(id)
        , m_access(access)
        , m_peer(peer)
        , m_name(name)
        , m_time(time)
    {}

    Start():
          m_software()
        , m_protocol()
        , m_features()
        , m_share_id()
        , m_access()
        , m_peer()
        , m_name()
        , m_time()
    {}

    bool operator==(const Start& o)
    {
        return std::tie(m_software, m_protocol, m_features, m_share_id, m_access, m_peer, m_name, m_time) ==
            std::tie(o.m_software, o.m_protocol, o.m_features, o.m_share_id, o.m_access, o.m_peer, o.m_name, o.m_time);
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
    std::string m_name;
    std::string m_time;
};

class Go: public MessageImpl<Go, MType::GO>
{
public:
    Go(
        const std::string& software,
        const int protocol,
        const std::vector<std::string>& features,
        const std::string& id,
        const std::string& access,
        const std::string& peer,
        const std::string& name,
        const std::string& time 
    ):
          m_software(software)
        , m_protocol(protocol)
        , m_features(features)
        , m_share_id(id)
        , m_access(access)
        , m_peer(peer)
        , m_name(name)
        , m_time(time)
    {}

    Go():
          m_software()
        , m_protocol()
        , m_features()
        , m_share_id()
        , m_access()
        , m_peer()
        , m_name()
        , m_time()
    {}

    bool operator==(const Go& o)
    {
        return std::tie(m_software, m_protocol, m_features, m_share_id, m_access, m_peer, m_name, m_time) ==
            std::tie(o.m_software, o.m_protocol, o.m_features, o.m_share_id, o.m_access, o.m_peer, o.m_name, o.m_time);
    }

    bool operator!=(const Go& o)
    {
        return ! (*this == o);
    }

    std::string m_software;
    int m_protocol = 0;
    std::vector<std::string> m_features;
    std::string m_share_id;
    std::string m_access;
    std::string m_peer;
    std::string m_name;
    std::string m_time;
};



class CannotStart: public MessageImpl<CannotStart, MType::CANNOT_START>
{
};

class GetUpdates: public MessageImpl<GetUpdates, MType::GET_UPDATES>
{
public:
    GetUpdates(const std::map<std::string, u64>& since):
        m_since(since)
    {
    }

    GetUpdates():
        m_since()
    {
    }

    std::map<std::string, u64> m_since;
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
    FileData(const std::string& checksum):
        m_checksum(checksum)
    {
        m_payload = true;
    }

    FileData():
        m_checksum()
    {
    }

    std::string m_checksum;
};

class NoSuchFile: public MessageImpl<NoSuchFile, MType::NO_SUCH_FILE>
{
public:
    NoSuchFile(const std::string& checksum):
        m_checksum(checksum)
    {
    }
    NoSuchFile():
        m_checksum()
    {}

    std::string m_checksum;
};



class Update: public MessageImpl<Update, MType::UPDATE>
{
public:
    Update():
        m_revision()
        , m_partial()
        , m_files()
    {}

    Update(long long revision):
        m_revision(revision)
        , m_partial()
        , m_files()
    {}
        

    Update(long long revision, bool partial, const std::vector<MFile>& files):
        m_revision(revision)
        , m_partial(partial)
        , m_files(files)
    {
    }

    /// peer revision number sending this Update
    u64 m_revision = 0;
    /// indicates that there are more Update messages comming...
    bool m_partial = false;
    std::vector<MFile> m_files;
};


} // end ns
} // end ns
} // end ns
