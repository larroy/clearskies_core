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

#ifdef __linux
    #define CS_PLATFORM_UNIX
    #define CS_PLATFORM_LINUX
#elif defined __APPLE__
    #include "TargetConditionals.h"
    #define CS_PLATFORM_UNIX
    #if TARGET_OS_MAC
        #define CS_PLATFORM_OSX
    #else
        #error This flavor of Apple OS is not supported yet
    #endif
#elif
    #error This platform is not supported yet
#endif

// gcc -E -dM - < /dev/null
//
#ifdef __GNUC__

#define likely(x) __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)
#define GCC_ATTRIBUTE(x) __attribute__((x))

#elif _WINDOWS

#define likely(x) (x)
#define unlikely(x) (x)
#define GCC_ATTRIBUTE(x)

#else

#define likely(x) (x)
#define unlikely(x) (x)
#define GCC_ATTRIBUTE(x)

#endif

#define UNUSED(x) ((void)(x))

#include "int_types.h"
#include "fs.hpp"
#include <memory>


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



