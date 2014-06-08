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
#include "vclock.hpp"
#include "config.hpp"
#include <limits>
#include <unordered_set>
#include "jsoncons/json.hpp"

using namespace std;

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

namespace cs
{

Vclock vclock_from_json(const std::string& s)
{
    Vclock result;
    const auto json = jsoncons::json::parse_string(s);
    for (auto i = json.begin_members(); i != json.end_members(); ++i)
        result.increment(i->first, i->second.as_ulonglong());
    return result;
}

std::string vclock_to_json(const Vclock& v)
{
    jsoncons::json json;
    for (const auto& x: v.get_values())
        json[x.first] = x.second;
    return json_2_str(json);
}

/**
 * A vclock desc is descendant from parent when All values(desc)  >= values(parent)
 */
bool Vclock::is_descendant(const Vclock& parent) const
{
    unordered_set<string> keys;
    for (const auto& x: m_clk)
        keys.insert(x.first);
    for (const auto& x: parent.m_clk)
        keys.insert(x.first);

    for (const auto& key: keys)
    {
        u64 desc_val = 0;
        u64 paren_val = 0;
        const auto di = m_clk.find(key);
        if (di != m_clk.end())
            desc_val = di->second;

        const auto pi = parent.m_clk.find(key);
        if (pi != parent.m_clk.end())
            paren_val = pi->second;

        if (desc_val < paren_val)
            return false;
    }
    return true;
}

u64 Vclock::operator[](const std::string& key) const
{
    const auto i = m_clk.find(key);
    if (i != m_clk.end())
        return i->second;
    return 0u;
}

void Vclock::increment(const std::string& key, u64 val)
{
    m_clk[key] += val;
}

void Vclock::merge(const Vclock& other)
{
    for (const auto& x: other.m_clk)
        m_clk[x.first] = std::max(m_clk[x.first], x.second);
}


} // end ns
