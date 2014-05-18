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

using namespace std;

namespace cs
{
namespace core
{
namespace msg
{


#define SC(X) static_cast<size_t>(X)
typedef std::array<const char*, SC(MType::MAX)> mtype_str_table_t;

namespace
{
mtype_str_table_t mtype_str_table_init()
{
    mtype_str_table_t res;
    res[SC(MType::UNKNOWN)] = "unknown";
    res[SC(MType::INTERNAL_SEND_START)] = "__internal_send_start";
    res[SC(MType::PING)] = "ping";
    res[SC(MType::START)] = "start";
    res[SC(MType::GO)] = "go";
    res[SC(MType::CANNOT_START)] = "cannot_start";
    res[SC(MType::GET_UPDATES)] = "get_updates";
    res[SC(MType::GET)] = "get";
    res[SC(MType::FILE_DATA)] = "file_data";
    res[SC(MType::NO_SUCH_FILE)] = "no_such_file";
    res[SC(MType::UPDATE)] = "update";
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

    if (type == "__internal_send_start")
        return MType::INTERNAL_SEND_START;

    if (type == "ping")
        return MType::PING;

    if (type == "start")
        return MType::START;

    if (type == "go")
        return MType::GO;

    if (type == "cannot_start")
        return MType::CANNOT_START;

    if (type == "get_updates")
        return MType::GET_UPDATES;

    if (type == "get")
        return MType::GET;

    if (type == "file_data")
        return MType::FILE_DATA;

    if (type == "no_such_file")
        return MType::NO_SUCH_FILE;

    if (type == "update")
        return MType::UPDATE;

    return MType::UNKNOWN;
}


size_t Message::MAX_SIZE = 1ULL << 24; // 16 MB

} // end ns
} // end ns
} // end ns

