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
    virtual ~CoderImpl() = 0;
    virtual std::unique_ptr<Message> decode_msg(bool payload, const char* msg, size_t len, std::string&& signature) = 0;
    virtual std::string encode_msg(const Message&) = 0;
};

namespace coder
{
/************************** JSON Implementation **************************/

namespace json
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
    msg.m_protocol = json["protocol"].as_vector<int>();
    msg.m_features = json["features"].as_vector<string>();
    msg.m_id = json["id"].as_string();
    msg.m_access = json["access"].as_string();
    msg.m_peer = json["peer"].as_string();
}

void decode(const jsoncons::json& json, CannotStart& msg)
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

void encode(const Greeting& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    json["software"] = msg.m_software;
    //json["protocol"] = json(msg.m_protocol.begin(), msg.m_protocol.end());
    //json["features"] = json(msg.m_features.begin(), msg.m_features.end());
}

void encode(const Start& msg, jsoncons::json& json)
{
    using namespace jsoncons;
    encode_type(msg, json);
    // FIXME
}

void encode(const CannotStart& msg, jsoncons::json& json)
{
    encode_type(msg, json);
}

// FIXME implement rest of messages


class JSONCoder: public CoderImpl, public ConstMessageVisitor
{
friend class Message;
public:
    JSONCoder():
        m_encoded_msg()
    {}


    std::unique_ptr<Message> decode_msg(bool payload, const char* msg, size_t len, std::string&& signature) override;
    std::string encode_msg(const Message&) override;

protected:
    void visit(const Unknown&) override;
    void visit(const InternalStart&) override;
    void visit(const Ping&) override;
    void visit(const Greeting&) override;
    void visit(const Start&) override;
    void visit(const CannotStart&) override;
    // FIXME implement rest of messages

private:
    std::string m_encoded_msg;
};


std::unique_ptr<Message> JSONCoder::decode_msg(bool payload, const char* msgdata, size_t len, std::string&& signature)
try
{
    const auto json = jsoncons::json::parse_string(string(msgdata, len));

    MType type = MType::UNKNOWN;
    if (json.has_member("type"))
        type = mtype_from_string(json["type"].as_string());

    switch(type)
    {
    case MType::INTERNAL_START:
    {
        auto msg = make_unique<InternalStart>();
        decode(json, *msg);
        return std::move(msg);
    }

    case MType::PING:
    {
        auto msg = make_unique<Ping>();
        decode(json, *msg);
        return std::move(msg);
    }

    case MType::GREETING:
    {
        auto msg = make_unique<Greeting>();
        decode(json, *msg);
        return std::move(msg);
    }

    case MType::START:
    {
        auto msg = make_unique<Start>();
        decode(json, *msg);
        return std::move(msg);
    }

    case MType::CANNOT_START:
    {
        auto msg = make_unique<CannotStart>();
        decode(json, *msg);
        return std::move(msg);
    }

    // FIXME implement rest of messages
    case MType::STARTTLS:
    case MType::IDENTITY:
    case MType::KEYS:
    case MType::KEYS_ACKNOWLEDGMENT:
    case MType::MANIFEST:
    case MType::GET_MANIFEST:
    case MType::MANIFEST_CURRENT:
    default:
    case MType::UNKNOWN:
        auto msg = make_unique<Unknown>();
        decode(json, *msg);
        return std::move(msg);
    }
    assert(0);
}
catch(const jsoncons::json_parse_exception& e)
{
    throw CoderError(fs("JSONCoder::decode JSON parse error:" << e.what()));
}
catch(const std::runtime_error& e)
{
    throw CoderError(fs("JSONCoder::decode runtime_error: " << e.what()));
}



std::string JSONCoder::encode_msg(const Message&)
{
    string result;
    return result;
}


void JSONCoder::visit(const Unknown&)
{
}

void JSONCoder::visit(const InternalStart&)
{
}

void JSONCoder::visit(const Ping&)
{
}

void JSONCoder::visit(const Greeting&)
{
}

void JSONCoder::visit(const Start&)
{
}

void JSONCoder::visit(const CannotStart&)
{
}


// FIXME implement rest of messages


} // end ns json
/************************** END JSON Implementation **************************/
} // end ns coder


Coder::Coder(CoderType type):
    m_p()
{
    switch(type)
    {
        case CoderType::JSON:
            m_p = make_unique<coder::json::JSONCoder>();
            break;

        default:
            assert(0);
    }
}

Coder::~Coder()
{
}


} // end ns
} // end ns
