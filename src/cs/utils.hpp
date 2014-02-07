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



namespace utils
{

/// @returns time in ISO 8601 format "YYYY-MM-DDThh:mm:ssZ"  (http://www.w4.org/TR/NOTE-datetime)
std::string isotime(std::time_t);


} // end ns
