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
    res[SC(MType::EMPTY)] = "empty";
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

Message::Message(const std::string& json):
    m_type(MType::UNKNOWN),
    m_payload(),
    m_signed(),
    m_json()
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




} // end ns
} // end ns
