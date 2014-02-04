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

#include "messagecoder.hpp"
#include "jsoncons/json.hpp"
#include <cassert>

using namespace std;

namespace cs
{
namespace message
{

class CoderImpl
{
public:
    virtual ~CoderImpl() {};
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

void decode(const jsoncons::json& json, InternalStart& msg)
{
}

void decode(const jsoncons::json& json, Ping& msg)
{
    msg.m_timeout = json["timeout"].as_int();
}

void decode(const jsoncons::json& json, Greeting& msg)
{
    msg.m_software = json["software"].as_string();
    msg.m_protocol = json["protocol"].as_vector<int>();
    msg.m_features = json["features"].as_vector<string>();
}

// TODO use a stringify macro to save some code
void decode(const jsoncons::json& json, Start& msg)
{

    msg.m_software = json["software"].as_string();
    msg.m_protocol = json["protocol"].as_int();
    msg.m_features = json["features"].as_vector<string>();
    msg.m_id = json["id"].as_string();
    msg.m_access = json["access"].as_string();
    msg.m_peer = json["peer"].as_string();
}

void decode(const jsoncons::json& json, CannotStart& msg)
{
}

void decode(const jsoncons::json& json, StartTLS& msg)
{
    msg.m_peer = json["peer"].as_string();
    msg.m_access = maccess_from_string(json["access"].as_string());
}

void decode(const jsoncons::json& json, Identity& msg)
{
    msg.m_name = json["name"].as_string();
    msg.m_time = json["time"].as_int();
}

void decode(const jsoncons::json& json, Keys& msg)
{
    msg.m_access = maccess_from_string(json["access"].as_string());
    msg.m_share_id = json["share_id"].as_string();

    auto ro = json["read_only"];
    msg.m_ro_psk = ro["psk"].as_string();
    msg.m_ro_rsa = ro["rsa"].as_string();

    auto rw = json["read_write"];
    msg.m_rw_public_rsa = rw["public_rsa"].as_string();
}

void decode(const jsoncons::json& json, KeysAcknowledgment& msg)
{
}

void decode(const jsoncons::json& json, Manifest& msg)
{
    msg.m_peer = json["peer"].as_string();
    msg.m_revision = json["revision"].as_longlong();

    // Read json file objects as MFiles
    for(int i = 0; i < json["files"].size(); i++) {
        auto j_file = json["files"][i];

        MFile file;
        file.m_path = j_file["path"].as_string();
        file.m_utime = j_file["utime"].as_double();

        if (j_file.has_member("deleted")) {
            file.m_deleted = j_file["deleted"].as_bool();
        } else {
            file.m_deleted = false;
        }

        if (!file.m_deleted) {
            file.m_size = j_file["size"].as_longlong();
            file.m_mtime = j_file["mtime"].as_vector<int>();
            file.m_mode = j_file["mode"].as_string();
            file.m_sha256 = j_file["sha256"].as_string();
        }

        msg.m_files.push_back(file);
    }
}

void decode(const jsoncons::json& json, GetManifest& msg)
{
    msg.m_revision = json["revision"].as_longlong();
}

void decode(const jsoncons::json& json, ManifestCurrent& msg)
{
}

void decode(const jsoncons::json& json, Get& msg)
{
    msg.m_path = json["path"].as_string();
    msg.m_range = json["range"].as_vector<long long>();
}

void decode(const jsoncons::json& json, FileData& msg)
{
    msg.m_path = json["path"].as_string();
    msg.m_range = json["range"].as_vector<long long>();
}

void decode(const jsoncons::json& json, Update& msg)
{
    msg.m_revision = json["revision"].as_longlong();

    // Read json file object as an MFile
    auto j_file = json["file"];

    msg.m_file.m_path = j_file["path"].as_string();
    msg.m_file.m_utime = j_file["utime"].as_double();

    if (j_file.has_member("deleted")) {
        msg.m_file.m_deleted = j_file["deleted"].as_bool();
    } else {
        msg.m_file.m_deleted = false;
    }

    if (!msg.m_file.m_deleted) {
        msg.m_file.m_size = j_file["size"].as_longlong();
        msg.m_file.m_mtime = j_file["mtime"].as_vector<int>();
        msg.m_file.m_mode = j_file["mode"].as_string();
        msg.m_file.m_sha256 = j_file["sha256"].as_string();
    }
}

void decode(const jsoncons::json& json, Move& msg)
{
    msg.m_revision = json["revision"].as_longlong();
    msg.m_source = json["source"].as_string();

    // Read json file object as an MFile
    auto j_file = json["destination"];

    msg.m_destination.m_path = j_file["path"].as_string();
    msg.m_destination.m_utime = j_file["utime"].as_double();
    msg.m_destination.m_size = j_file["size"].as_longlong();
    msg.m_destination.m_mtime = j_file["mtime"].as_vector<int>();
    msg.m_destination.m_mode = j_file["mode"].as_string();
    msg.m_destination.m_sha256 = j_file["sha256"].as_string();
    msg.m_destination.m_deleted = false;
}

void decode(const jsoncons::json& json, Max& msg)
{
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

void encode(const InternalStart& msg, jsoncons::json& json)
{
    encode_type(msg, json);
}

void encode(const Ping& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["timeout"] = msg.m_timeout;
}

template<class It>
jsoncons::json to_array(It begin, It end)
{
    auto result = jsoncons::json::make_array();
    for (auto i = begin; i != end; ++i)
        result.add(jsoncons::json(*i));
    return result;
}

void encode(const Greeting& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["software"] = msg.m_software;
    //json["protocol"] = to_array(msg.m_protocol.begin(), msg.m_protocol.end());
    json["protocol"] = jsoncons::json(msg.m_protocol.begin(), msg.m_protocol.end());
    json["features"] = jsoncons::json(msg.m_features.begin(), msg.m_features.end());
}

void encode(const Start& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["software"] = msg.m_software;
    json["protocol"] = to_string(msg.m_protocol);
    json["features"] = jsoncons::json(msg.m_features.begin(), msg.m_features.end());
    json["id"] = msg.m_id;
    json["access"] = msg.m_access;
    json["peer"] = msg.m_peer;
}

void encode(const CannotStart& msg, jsoncons::json& json)
{
    encode_type(msg, json);
}

void encode(const StartTLS& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["peer"] = msg.m_peer;
    json["access"] = maccess_to_string(msg.m_access);
}

void encode(const Identity& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["name"] = msg.m_name;
    json["time"] = to_string(msg.m_time);
}

void encode(const Keys& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["access"] = maccess_to_string(msg.m_access);
    json["share_id"] = msg.m_share_id;

    jsoncons::json ro;
    ro["psk"] = msg.m_ro_psk;
    ro["rsa"] = msg.m_ro_rsa;
    json["read_only"] = ro;

    jsoncons::json rw;
    rw["public_rsa"] = msg.m_rw_public_rsa;
    json["read_write"] = rw;
}

void encode(const KeysAcknowledgment& msg, jsoncons::json& json)
{
    assert(0);
}

void encode(const Manifest& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["peer"] = msg.m_peer;
    json["revision"] = msg.m_revision;

    std::vector<jsoncons::json> j_files;
    for (MFile file: msg.m_files) {
        jsoncons::json j_file;
        j_file["path"] = file.m_path;
        j_file["utime"] = file.m_utime;

        if (!file.m_deleted) {
            j_file["size"] = file.m_size;
            j_file["mtime"] = jsoncons::json(file.m_mtime.begin(), file.m_mtime.end());
            j_file["mode"] = file.m_mode;
            j_file["sha256"] = file.m_sha256;
        } else {
            j_file["deleted"] = file.m_deleted;
        }

        j_files.push_back(j_file);
    }

    json["files"] = jsoncons::json(j_files.begin(), j_files.end());
}

void encode(const GetManifest& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["revision"] = msg.m_revision;
}

void encode(const ManifestCurrent& msg, jsoncons::json& json)
{
    assert(0);
}

void encode(const Get& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["path"] = msg.m_path;
    json["range"] = jsoncons::json(msg.m_range.begin(), msg.m_range.end());
}

void encode(const FileData& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["path"] = msg.m_path;
    json["range"] = jsoncons::json(msg.m_range.begin(), msg.m_range.end());
}

void encode(const Update& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["revision"] = msg.m_revision;

    jsoncons::json j_file;
    j_file["path"] = msg.m_file.m_path;
    j_file["utime"] = msg.m_file.m_utime;

    if (!msg.m_file.m_deleted) {
        j_file["size"] = msg.m_file.m_size;
        j_file["mtime"] = jsoncons::json(msg.m_file.m_mtime.begin(), msg.m_file.m_mtime.end());
        j_file["mode"] = msg.m_file.m_mode;
        j_file["sha256"] = msg.m_file.m_sha256;
    } else {
        j_file["deleted"] = msg.m_file.m_deleted;
    }

    json["file"] = j_file;
}

void encode(const Move& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["revision"] = msg.m_revision;

    jsoncons::json j_file;
    j_file["path"] = msg.m_destination.m_path;
    j_file["utime"] = msg.m_destination.m_utime;
    j_file["size"] = msg.m_destination.m_size;
    j_file["mtime"] = jsoncons::json(msg.m_destination.m_mtime.begin(), msg.m_destination.m_mtime.end());
    j_file["mode"] = msg.m_destination.m_mode;
    j_file["sha256"] = msg.m_destination.m_sha256;

    json["file"] = j_file;
}

void encode(const Max& msg, jsoncons::json& json)
{
    assert(0);
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
    void visit(const InternalStart&) override;
    void visit(const Ping&) override;
    void visit(const Greeting&) override;
    void visit(const Start&) override;
    void visit(const CannotStart&) override;
    void visit(const StartTLS&) override;
    void visit(const Identity&) override;
    void visit(const Keys&) override;
    void visit(const KeysAcknowledgment&) override;
    void visit(const Manifest&) override;
    void visit(const GetManifest&) override;
    void visit(const ManifestCurrent&) override;
    void visit(const Get&) override;
    void visit(const FileData&) override;
    void visit(const Update&) override;
    void visit(const Move&) override;
    void visit(const Max&) override;

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
    case MType::INTERNAL_START:
    {
        auto xmsg = make_unique<InternalStart>();
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

    case MType::GREETING:
    {
        auto xmsg = make_unique<Greeting>();
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

    case MType::CANNOT_START:
    {
        auto xmsg = make_unique<CannotStart>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::STARTTLS:
    {
        auto xmsg = make_unique<StartTLS>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::IDENTITY:
    {
        auto xmsg = make_unique<Identity>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::KEYS:
    {
        auto xmsg = make_unique<Keys>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::KEYS_ACKNOWLEDGMENT:
    {
        auto xmsg = make_unique<KeysAcknowledgment>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::MANIFEST:
    {
        auto xmsg = make_unique<Manifest>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::GET_MANIFEST:
    {
        auto xmsg = make_unique<GetManifest>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::MANIFEST_CURRENT:
    {
        auto xmsg = make_unique<ManifestCurrent>();
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

    case MType::UPDATE:
    {
        auto xmsg = make_unique<Update>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::MOVE:
    {
        auto xmsg = make_unique<Move>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    case MType::MAX:
    {
        auto xmsg = make_unique<Max>();
        decode(json, *xmsg);
        msg = move(xmsg);
        break;
    }

    // FIXME implement rest of messages

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
    char prefix = 0;
    if (! msg.payload() && ! msg.signature())
        prefix = 'm';
    else if (msg.payload() && ! msg.signature())
        prefix = '!';
    else if (! msg.payload() &&  msg.signature())
        prefix = 's';
    else if (msg.payload() &&  msg.signature())
        prefix = '$';

    // m3:{}\n
    msg.accept(*this);
    ostringstream result;
    result << prefix;
    assert(m_encoded_msg.size() <= Message::MAX_SIZE);
    result << m_encoded_msg.size();
    result << ':';
    result << m_encoded_msg;
    result << '\n';
    if (msg.signature())
        result << msg.m_signature << '\n';

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

void JSONCoder::visit(const InternalStart& x)
{
    ENCXX;
}

void JSONCoder::visit(const Ping& x)
{
    ENCXX;
}

void JSONCoder::visit(const Greeting& x)
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

void JSONCoder::visit(const StartTLS& x)
{
    ENCXX;
}

void JSONCoder::visit(const Identity& x)
{
    ENCXX;
}

void JSONCoder::visit(const Keys& x)
{
    ENCXX;
}

void JSONCoder::visit(const KeysAcknowledgment& x)
{
    ENCXX;
}

void JSONCoder::visit(const Manifest& x)
{
    ENCXX;
}

void JSONCoder::visit(const GetManifest& x)
{
    ENCXX;
}

void JSONCoder::visit(const ManifestCurrent& x)
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

void JSONCoder::visit(const Update& x)
{
    ENCXX;
}

void JSONCoder::visit(const Move& x)
{
    ENCXX;
}

void JSONCoder::visit(const Max& x)
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
