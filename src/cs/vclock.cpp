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
#include <gmpxx.h>


using namespace std;

namespace cs
{


class VclockImpl
{
public:
    VclockImpl():
        m_clk()
    {}

    VclockImpl(const std::map<std::string, std::string>& initial);
    bool is_descendant(const Vclock& other) const;
    std::string operator[](const std::string& key) const;
    std::map<std::string, std::string> get_values() const;
    void increment(const std::string& key, u32 val = 1);

private:
    std::map<std::string, mpz_class> m_clk;
};


VclockImpl::VclockImpl(const std::map<std::string, std::string>& initial):
    m_clk()
{
    for (const auto& x: initial)
        m_clk.emplace(x.first, move(mpz_class(x.second)));
}

/**
 * A vclock desc is descendant from parent when All values(desc)  >= values(parent)
 */
bool VclockImpl::is_descendant(const Vclock& parent_) const
{
    const VclockImpl& parent = *parent_.m_p;

    unordered_set<string> keys;
    for (const auto& x: m_clk)
        keys.insert(x.first);
    for (const auto& x: parent.m_clk)
        keys.insert(x.first);

    for (const auto& key: keys)
    {
        mpz_class desc_val = 0;
        mpz_class paren_val = 0;
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

std::string VclockImpl::operator[](const std::string& key) const
{
    const auto i = m_clk.find(key);
    if (i != m_clk.end())
        return i->second.get_str();
    return "0";
}

std::map<std::string, std::string> VclockImpl::get_values() const
{
    map<string, string> result;
    for (const auto& x: m_clk)
        result.emplace(x.first, x.second.get_str());
    return result;
}

void VclockImpl::increment(const std::string& key, u32 val)
{
    auto i = m_clk.find(key);
    if (i != m_clk.end())
        i->second += val;
    else
        m_clk.emplace(key, val);
}

Vclock::Vclock():
    m_p(make_unique<VclockImpl>())
{}

Vclock::Vclock(const std::map<std::string, std::string>& initial):
    m_p(make_unique<VclockImpl>(initial))
{
}

Vclock::~Vclock() = default;
Vclock::Vclock(Vclock&&) = default;
Vclock& Vclock::operator=(Vclock&&) = default;

bool Vclock::is_descendant(const Vclock& other) const
{
    return m_p->is_descendant(other);
}

std::string Vclock::operator[](const std::string& key) const
{
    return (*m_p)[key];
}

std::map<std::string, std::string> Vclock::get_values() const
{
    return m_p->get_values();
}

void Vclock::increment(const std::string& key, u32 val)
{
    m_p->increment(key, val);
}


} // end ns
