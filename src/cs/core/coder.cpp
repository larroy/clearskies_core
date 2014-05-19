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

#include "coder.hpp"
#include "jsoncons/json.hpp"
#include "cs/obytestream.hpp"
#include <cassert>

using namespace std;

namespace cs
{
namespace core
{
namespace msg
{

class CoderImpl
{
public:
    virtual ~CoderImpl() {};
    CoderImpl(CoderImpl&&) = default;
    CoderImpl() = default;
    virtual std::unique_ptr<Message> decode_msg(bool, const char*, size_t, const char*, size_t) = 0;
    virtual std::string encode_msg(const Message&) = 0;
};

namespace coder
{
/************************** JSON Implementation **************************/

namespace jsoni
{

/*** decode json -> msg ***/

void decode(const jsoncons::json& json, Unknown& msg)
{
    // save the json message recoded for inspection
    ostringstream json_os;
    jsoncons::json_serializer serializer(json_os, true);
    json.to_stream(serializer);
    msg.m_content = json_os.str();
}

void decode(const jsoncons::json& json, InternalSendStart& msg)
{
    msg.m_share_id = json["share_id"].as_string();
}

void decode(const jsoncons::json& json, Ping& msg)
{
    msg.m_timeout = json["timeout"].as_int();
}

void decode(const jsoncons::json& json, Start& msg)
{
    msg.m_software = json["software"].as_string();
    msg.m_protocol = json["protocol"].as_int();
    msg.m_features = json["features"].as_vector<string>();
    msg.m_share_id = json["id"].as_string();
    msg.m_access = json["access"].as_string();
    msg.m_peer = json["peer"].as_string();
    msg.m_name = json["name"].as_string();
    msg.m_time = json["time"].as_string();
}

void decode(const jsoncons::json& json, Go& msg)
{
    msg.m_software = json["software"].as_string();
    msg.m_protocol = json["protocol"].as_int();
    msg.m_features = json["features"].as_vector<string>();
    msg.m_share_id = json["id"].as_string();
    msg.m_access = json["access"].as_string();
    msg.m_peer = json["peer"].as_string();
    msg.m_name = json["name"].as_string();
    msg.m_time = json["time"].as_string();
}

void decode(const jsoncons::json& json, CannotStart& msg)
{
}

void decode(const jsoncons::json& json, GetUpdates& msg)
{
    auto since = json["since"];
    for (auto i = since.begin_members(); i != since.end_members(); ++i)
        msg.m_since.insert(make_pair(i->first, i->second.as_ulonglong()));
}


void decode(const jsoncons::json& json, Get& msg)
{
    msg.m_checksum = json["checksum"].as_string();
}

void decode(const jsoncons::json& json, FileData& msg)
{
    msg.m_checksum = json["checksum"].as_string();
}

void decode(const jsoncons::json& json, NoSuchFile& msg)
{
    msg.m_checksum = json["checksum"].as_string();
}

void decode(const jsoncons::json& json, Update& msg)
{
    msg.m_revision = json["revision"].as_ulonglong();
    msg.m_partial = json.has_member("partial") && json["partial"].as_bool() == true;

    auto files = json["files"];
    for (auto i = files.begin_elements(); i != files.end_elements(); ++i)
    {
        const auto& file = *i;
        MFile mfile;
        auto paths = file["paths"];
        for (auto pi = paths.begin_elements(); pi != paths.end_elements(); ++pi)
            mfile.paths.emplace_back(pi->as_string());

        if (mfile.paths.empty())
            throw std::runtime_error("mfile paths is empty");

        mfile.last_changed_by = file["last_changed_by"].as_string();
        mfile.last_changed_rev = file["last_changed_rev"].as_ulonglong();
        mfile.mtime = file["mtime"].as_string();
        mfile.size = file["size"].as_ulonglong();
        mfile.mode = file["size"].as_ulong();
        mfile.deleted = file.has_member("deleted") && file["deleted"].as_bool() == true;
    }
}


/*** encode msg -> json ***/

void encode_type(const Message& msg, jsoncons::json& json)
{
    namespace jc = jsoncons;
    json = jc::json::an_object;
    json["type"] = mtype_to_string(msg.type());
}

void encode(const Unknown& msg, jsoncons::json& json)
{
    assert(0);
}

void encode(const InternalSendStart& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["share_id"] = msg.m_share_id;
}

void encode(const Ping& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["timeout"] = msg.m_timeout;
}

void encode(const Start& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["software"] = msg.m_software;
    json["protocol"] = msg.m_protocol;
    json["features"] = jsoncons::json(msg.m_features.begin(), msg.m_features.end());
    json["id"] = msg.m_share_id;
    json["access"] = msg.m_access;
    json["peer"] = msg.m_peer;
    json["name"] = msg.m_name;
    json["time"] = msg.m_time;
}

void encode(const Go& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["software"] = msg.m_software;
    json["protocol"] = msg.m_protocol;
    json["features"] = jsoncons::json(msg.m_features.begin(), msg.m_features.end());
    json["id"] = msg.m_share_id;
    json["access"] = msg.m_access;
    json["peer"] = msg.m_peer;
    json["name"] = msg.m_name;
    json["time"] = msg.m_time;
}

void encode(const CannotStart& msg, jsoncons::json& json)
{
    encode_type(msg, json);
}

void encode(const GetUpdates& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["since"] = json::an_object;
    for (const auto& x: msg.m_since)
        json["since"][x.first] = x.second;
}

void encode(const Get& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["checksum"] = msg.m_checksum;
}

void encode(const FileData& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["checksum"] = msg.m_checksum;
}

void encode(const NoSuchFile& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["checksum"] = msg.m_checksum;
}


void encode(const Update& msg, jsoncons::json& jmsg)
{
    using namespace jsoncons;
    encode_type(msg, jmsg);
    jmsg["revision"] = msg.m_revision;
    jmsg["partial"] = msg.m_partial;
    jmsg["files"] = json::make_array();
    for (const auto& mfile: msg.m_files)
    {
        json file;
        file["checksum"] = mfile.checksum;
        file["paths"] = json(mfile.paths.begin(), mfile.paths.end());
        file["last_changed_by"] = mfile.last_changed_by;
        file["last_changed_rev"] = mfile.last_changed_rev;
        file["mtime"] = mfile.mtime;
        file["size"] = mfile.size;
        file["mode"] = mfile.mode;
        file["deleted"] = mfile.deleted;
        jmsg["files"].add(move(file));
    }
}

class JSONCoder: public CoderImpl, public ConstMessageVisitor
{
friend class Message;
public:
    JSONCoder():
        m_encoded_msg()
    {}


    std::unique_ptr<Message> decode_msg(bool, const char*, size_t, const char*, size_t) override;
    std::string encode_msg(const Message&) override;
    void reset()
    {
        m_encoded_msg.clear();
    }

protected:
    void visit(const Unknown&) override;
    void visit(const InternalSendStart&) override;
    void visit(const Ping&) override;
    void visit(const Start&) override;
    void visit(const CannotStart&) override;
    void visit(const Go&) override;
    void visit(const GetUpdates&) override;
    void visit(const Get&) override;
    void visit(const FileData&) override;
    void visit(const NoSuchFile&) override;
    void visit(const Update&) override;

private:
    std::string m_encoded_msg;
};


std::unique_ptr<Message> JSONCoder::decode_msg(bool payload, const char* encoded, size_t encoded_sz, const char* signature, size_t signature_sz)
try
{
    const auto json = jsoncons::json::parse_string(string(encoded, encoded_sz));

    MType type = MType::UNKNOWN;
    if (json.has_member("type"))
        type = mtype_from_string(json["type"].as_string());

    unique_ptr<Message> msg;
    switch(type)
    {
    case MType::INTERNAL_SEND_START:
    {
        auto xmsg = make_unique<InternalSendStart>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::PING:
    {
        auto xmsg = make_unique<Ping>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::START:
    {
        auto xmsg = make_unique<Start>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::GO:
    {
        auto xmsg = make_unique<Go>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::CANNOT_START:
    {
        auto xmsg = make_unique<CannotStart>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::GET_UPDATES:
    {
        auto xmsg = make_unique<GetUpdates>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::GET:
    {
        auto xmsg = make_unique<Get>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::FILE_DATA:
    {
        auto xmsg = make_unique<FileData>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::NO_SUCH_FILE:
    {
        auto xmsg = make_unique<NoSuchFile>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::UPDATE:
    {
        auto xmsg = make_unique<Update>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }


    // Add additional message types here

    default:
    case MType::UNKNOWN:
        auto xmsg = make_unique<Unknown>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    msg->m_payload = payload;
    msg->m_signature.assign(signature, signature_sz);
    return std::move(msg);
}
catch(const jsoncons::json_exception& e)
{
    throw CoderError(fs("JSONCoder::decode JSON parse error: " << e.what()));
}
catch(const std::runtime_error& e)
{
    throw CoderError(fs("JSONCoder::decode runtime_error: " << e.what()));
}
catch(const std::exception & e)
{
    throw CoderError(fs("JSONCoder::decode exception: " << e.what()));
}


namespace
{

std::string json_2_str(const jsoncons::json& json)
{
    ostringstream json_os;
    jsoncons::json_serializer serializer(json_os, false); // no indent
    json.to_stream(serializer);
    return json_os.str();
}

}


std::string JSONCoder::encode_msg(const Message& msg)
{
    using namespace cs::io;
    char prefix = 0;
    if (! msg.m_payload && ! msg.signature())
        prefix = 'm';
    else if (msg.m_payload && ! msg.signature())
        prefix = '!';
    else if (! msg.m_payload &&  msg.signature())
        prefix = 's';
    else if (msg.m_payload &&  msg.signature())
        prefix = '$';

    msg.accept(*this); // fills m_encoded_msg with the selected encoder
    ostringstream result;
    result << prefix;
    Obytestream ob;
    assert(m_encoded_msg.size() <= Message::MAX_SIZE);
    ob.write<u32>(m_encoded_msg.size());
    result << ob.m_buff;
    result << ':';
    result << m_encoded_msg;
    if (msg.signature())
    {
        Obytestream obs;
        obs.write<u32>(msg.m_signature.size());
        result << obs.m_buff;
        result << ':';
        result << msg.m_signature;
    }

    /******/
    // reset m_encoded_msg
    reset();
    /******/

    return result.str();
}

#define ENCXX\
    do {\
        jsoncons::json json;\
        encode(x, json);\
        m_encoded_msg = json_2_str(json);\
    } while(0);

void JSONCoder::visit(const Unknown&)
{
    assert(0);
}

void JSONCoder::visit(const InternalSendStart& x)
{
    ENCXX;
}

void JSONCoder::visit(const Ping& x)
{
    ENCXX;
}

void JSONCoder::visit(const Start& x)
{
    ENCXX;
}

void JSONCoder::visit(const CannotStart& x)
{
    ENCXX;
}

void JSONCoder::visit(const Go& x)
{
    ENCXX;
}



void JSONCoder::visit(const GetUpdates& x)
{
    ENCXX;
}

void JSONCoder::visit(const Get& x)
{
    ENCXX;
}

void JSONCoder::visit(const FileData& x)
{
    ENCXX;
}

void JSONCoder::visit(const NoSuchFile& x)
{
    ENCXX;
}

void JSONCoder::visit(const Update& x)
{
    ENCXX;
}



} // end ns json
/************************** END JSON Implementation **************************/
} // end ns coder


Coder::Coder(CoderType type):
    m_p()
{
    switch(type)
    {
        case CoderType::JSON:
            m_p = make_unique<coder::jsoni::JSONCoder>();
            break;

        default:
            assert(0);
    }
}

// needs to be defined here because of m_p and CoderImpl fwd declaration
Coder::~Coder()
{
}


Coder::Coder(Coder&&) = default;
Coder& Coder::operator=(Coder&&) = default;


std::unique_ptr<Message> Coder::decode_msg(bool payload, const char* encoded, size_t encoded_sz, const char* signature, size_t signature_sz)
{
    return m_p->decode_msg(payload, encoded, encoded_sz, signature, signature_sz);
}

std::string Coder::encode_msg(const Message& m) const
{
    return m_p->encode_msg(m);
}



} // end ns
} // end ns
} // end ns
