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
#include <sstream>
#include <memory>
#include <ctime>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <random>

#include "int_types.h"
#include "boost_fs_fwd.hpp"
#include "config.hpp"

#define DEFINE_RE_EXCEPTION(CLASS_NAME)\
class CLASS_NAME: public std::runtime_error\
{ public: explicit CLASS_NAME(const std::string& what): std::runtime_error(what) {} };

namespace cs
{
namespace utils
{

/// @returns time in ISO 8601 format "YYYY-MM-DDThh:mm:ssZ"  (http://www.w4.org/TR/NOTE-datetime)
std::string isotime(std::time_t);

std::string bin_to_hex(const void* b, size_t sz);
inline std::string bin_to_hex(const std::string& s)
{
    return utils::bin_to_hex(s.c_str(), s.size());
}

template<typename OUT_T>
OUT_T hex_to_bin(const std::string& xs)
{
    OUT_T result;
    result.reserve(xs.size() * 2);
    u8 nibble = 0;
    for (auto x: xs)
    {
        assert(isxdigit(x));
        if (isdigit(x))
            x -= '0';
        else if (isalpha(x))
        {
            if (islower(x))
                x -= 'a';
            else
                x -= 'A';
            x += 10;
        }
        if (nibble == 0)
        {
            result.push_back(x << 4);
            ++nibble;
        }
        else
        {
            result.back() |= x;
            nibble = 0;
        }
    }
    return result;
}


std::string random_bytes(size_t);

/**
 * create a temporary directory in TMPDIR, TMP, TEMP, TEMPDIR. If none of these are found, "/tmp".
 */
inline bfs::path tmpdir(const std::string& pattern = "clearskies-%%%%-%%%%-%%%%-%%%%")
{
    return bfs::temp_directory_path() / bfs::unique_path(pattern);
}


struct Tmpdir
{
    Tmpdir():
        path(tmpdir())
    {
        assert(! bfs::exists(path));
        bfs::create_directory(path);
        assert(bfs::exists(path));
    }

    ~Tmpdir()
    {
        bfs::remove_all(path);
    }

    Tmpdir(const Tmpdir&) = delete;
    Tmpdir& operator=(const Tmpdir&) = delete;
    Tmpdir(Tmpdir&&) = delete;
    Tmpdir& operator=(Tmpdir&&) = delete;

    bfs::path path;
};

class GCC_ATTRIBUTE(unused) ScopeGuardBase : private boost::noncopyable
{
public:
    ScopeGuardBase():
        m_enabled(true)
    {}

    void disable() const
    {
        m_enabled = false;
    }
protected:
    mutable bool m_enabled;
};

typedef GCC_ATTRIBUTE(unused) const ScopeGuardBase& ScopeGuard;

template<typename T>
class ScopeGuardTemplate : public ScopeGuardBase
{
public:
    explicit ScopeGuardTemplate(T&& function):
        m_function(function)
    {
    }

    ScopeGuardTemplate(ScopeGuardTemplate&& other)
        :m_function(other.m_function)
    {
        other.disable();
    }

    ~ScopeGuardTemplate()
    {
        if (m_enabled)
            m_function();
    }
private:

    ScopeGuardTemplate(const ScopeGuardTemplate& other);

    ScopeGuardTemplate(T& function); // prevent modifying the lambda after creating the guard

    T m_function;

};

template<typename T>
static inline ScopeGuardTemplate<T> make_scope_guard(T&& function)
{
    return ScopeGuardTemplate<T>(std::forward<T>(function));
}


template<typename T>
std::vector<T> random_uniform_vector(size_t count)
{
    std::vector<T> result;
    std::random_device rng;
    std::uniform_int_distribution<T> udist;
    generate_n(back_inserter(result), count, [&] { return udist(rng); });
    return result;
}



} // end ns
} // end ns
