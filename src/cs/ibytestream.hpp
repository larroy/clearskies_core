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
#include <type_traits>
#include <cassert>
#include "config.hpp"

namespace cs
{
namespace io
{

class Ibytestream
{
public:
    Ibytestream(u8 const* begin, u8 const* end):
        m_next(begin)
        , m_end(end)
    {}

    template<typename T>
    T read()
    {
        static_assert(std::is_integral<T>::value, "argument must be integral");
        assert(m_next + sizeof(T) <= m_end);
        T result = 0;
        for (i8 i = sizeof(T) - 1; i >= 0; --i)
        {
            result |= static_cast<T>(*m_next++);
            result <<= 8;
        }

        return result;
    }

    u8 const* m_next;
    u8 const* const m_end;
};

}
}

