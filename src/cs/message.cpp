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
#include "message.hpp"
#include <sstream>
#include <cassert>
#include <array>
#include "jsoncons/json_serializer.hpp"

using namespace std;

namespace cs
{
namespace message
{


#define SC(X) static_cast<size_t>(X)
typedef std::array<const char*, SC(MType::MAX)> mtype_str_table_t;

namespace
{
mtype_str_table_t mtype_str_table_init()
{
    mtype_str_table_t res;
    res[SC(MType::UNKNOWN)] = "unknown";
    res[SC(MType::INTERNAL_START)] = "__internal_start";
    res[SC(MType::PING)] = "ping";
    res[SC(MType::GREETING)] = "greeting";
    res[SC(MType::START)] = "start";
    res[SC(MType::CANNOT_START)] = "cannot_start";
    res[SC(MType::STARTTLS)] = "starttls";
    res[SC(MType::IDENTITY)] = "identity";
    res[SC(MType::KEYS)] = "keys";
    res[SC(MType::KEYS_ACKNOWLEDGMENT)] = "keys_acknowledgment";
    res[SC(MType::MANIFEST)] = "manifest";
    res[SC(MType::GET_MANIFEST)] = "get_manifest";
    res[SC(MType::GET)] = "get";
    res[SC(MType::FILE_DATA)] = "file_data";
    res[SC(MType::UPDATE)] = "update";
    res[SC(MType::MOVE)] = "move";
    return res;
}
} // end anon ns

std::string mtype_to_string(MType type)
{
    static const mtype_str_table_t mtype_str_table = mtype_str_table_init();
    return mtype_str_table[SC(type)];
}
#undef SC

MType mtype_from_string(const std::string& type)
{
    if (type == "unknown")
        return MType::UNKNOWN;

    if (type == "__internal_start")
        return MType::INTERNAL_START;

    if (type == "ping")
        return MType::PING;

    if (type == "greeting")
        return MType::GREETING;

    if (type == "start")
        return MType::START;

    if (type == "cannot_start")
        return MType::CANNOT_START;

    if (type == "starttls")
        return MType::STARTTLS;

    if (type == "identity")
        return MType::IDENTITY;

    if (type == "keys")
        return MType::KEYS;

    if (type == "keys_acknowledgment")
        return MType::KEYS;

    if (type == "manifest")
        return MType::MANIFEST;

    if (type == "get_manifest")
        return MType::GET_MANIFEST;

    if (type == "manifest_current")
        return MType::MANIFEST_CURRENT;

    if (type == "get")
        return MType::GET;

    if (type == "file_data")
        return MType::FILE_DATA;

    if (type == "update")
        return MType::UPDATE;

    if (type == "move")
        return MType::MOVE;

    return MType::UNKNOWN;
}


size_t Message::MAX_SIZE = 1ULL << 20;

Message::Message(const std::string& json, bool payload,  const std::string& signature):
      m_type(MType::UNKNOWN)
    , m_has_payload(payload)
    , m_json()
    , m_signature(signature)
{
    // translate jsoncons::json_parse_exception to MessageError
    try
    {
        m_json = jsoncons::json::parse_string(json);
    }
    catch(const jsoncons::json_parse_exception& e)
    {
        throw MessageError(fs("json parse error:" << e.what()));
    }
    // set type from json
    if (m_json.has_member("type"))
        m_type = mtype_from_string(m_json["type"].as_string());
}


std::string Message::serialize()
{
    ostringstream os;
    ostringstream json_os;
    jsoncons::json_serializer serializer(json_os, false);

    fill_json();
    m_json.to_stream(serializer);
    const string& json_str = json_os.str();

    // m2{}\n
    os << prefix();
    assert(json_str.size() <= MAX_SIZE);
    os << json_str.size();
    os << json_str;
    os << '\n';

    return os.str();
}

void Message::fill_json()
{
    using namespace jsoncons;
    m_json = json::an_object;
    m_json["type"] = mtype_to_string(type());
}


char Message::prefix() const
{
    char res = 'm';
    if (m_has_payload && ! m_signature.empty())
        res = '$';
    else if (m_has_payload && m_signature.empty())
        res = '!';
    else if (! m_has_payload && m_signature.empty())
        res = 'm';
    else if (! m_has_payload && ! m_signature.empty())
        res = 's';
    return res;
}


void Ping::fill_json()
{
    using namespace jsoncons;
    m_json = json::an_object;
    m_json["type"] = mtype_to_string(type());
    m_json["timeout"] = m_timeout;
}

} // end ns
} // end ns

