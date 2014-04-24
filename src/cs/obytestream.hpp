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
#include "cs/config.hpp"

namespace cs
{
namespace io
{

class Obytestream
{
public:

    /// W type to write as
    /// T argument type
    template<typename W, typename T>
    void write(T x)
    {
        static_assert(std::is_integral<W>::value && std::is_integral<T>::value, "argument must be integral");
        static_assert(sizeof(W) <= sizeof(T), "Write size has to be <= of original type");
        W w = x; 
        for (i8 i = sizeof(W) - 1; i >= 0; --i) 
            m_buff.push_back(static_cast<char>(w >> i));
    }

    std::string m_buff;
};

}
}

