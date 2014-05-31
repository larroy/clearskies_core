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
#include <map>
#include <memory>
#include <string>


namespace cs
{


/**
 * A Vclock is descendant of another when changes are a consequence of another, thus the changes are
 * newer and not in conflict. Otherwise the changes are "outdated" and are in conflict.
 *
 * Limitations:
 *
 * To handle integer overflows we assume that vector clock with a value of 0 is a descendant of
 * std::numeric_limits<u32>::max so we can wrap around. This means that there's a very small chance
 * that a very old change will appear as new if it's compared against the max value.
 */
class Vclock
{
public:
    Vclock():
        m_clk()
    {}

    Vclock(const std::map<std::string, u64>& clk):
        m_clk(clk)
    {}

    /// @returns true if this clock is a descendant from @param other
    bool is_descendant(const Vclock& other) const;

    bool is_ancestor(const Vclock& other) const
    {
        return other.is_descendant(*this);
    }

    /**
     * access the version field at clock given by @param key, if key doesn't exists is assumed to
     * have value of 0
     * @returns an arbitrary precission int in base10
     */
    u64 operator[](const std::string& key) const;

    const std::map<std::string, u64>& get_values() const
    {
        return m_clk;
    }


    /// increment clock @param key by @param val
    void increment(const std::string& key, u64 val = 1);

    bool operator==(const Vclock& o) const
    {
        return std::tie(m_clk) == std::tie(o.m_clk);
    }

private:
    std::map<std::string, u64> m_clk;
};


Vclock vclock_from_json(const std::string&);
std::string vclock_to_json(const Vclock&);

} // end ns


