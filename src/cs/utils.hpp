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
#include "int_types.h"

namespace std
{
// Forgotten stuff in the C++11 standard:

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template< class C >
auto cbegin( C& c ) -> decltype(c.cbegin())
{
    return c.cbegin();
}

template< class C >
auto cbegin( const C& c ) -> decltype(c.cbegin())
{
    return c.cbegin();
}

template< class C >
auto cend( C& c ) -> decltype(c.cend())
{
    return c.cend();
}

template< class C >
auto cend( const C& c ) -> decltype(c.cend())
{
    return c.cend();
}


} // end ns





/// Formatted string, allows to use stream operators and returns a std::string with the resulting format
#define fs(x) \
   (static_cast<const std::ostringstream&>(((*std::make_unique<std::ostringstream>().get()) << x)).str ())


/// Format error
#define fe(x) \
   (static_cast<const std::ostringstream&>(((*std::make_unique<std::ostringstream>().get()) << "ERROR: "<< __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  x)).str ())



namespace cs
{
namespace utils
{

/// @returns time in ISO 8601 format "YYYY-MM-DDThh:mm:ssZ"  (http://www.w4.org/TR/NOTE-datetime)
std::string isotime(std::time_t);

std::string bin_to_hex(const void* b, size_t sz);
inline std::string bin_to_hex(const std::string& s)
{
    return utils::bin_to_hex(reinterpret_cast<const u8*>(s.c_str()), s.size());
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


} // end ns
} // end ns
