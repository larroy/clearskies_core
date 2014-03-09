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
#include "config.hpp"

namespace cs
{


struct ServerInfo
{
    ServerInfo():
        m_name()
        , m_software()
        , m_protocol()
        , m_features()
    {}

    std::string m_name;
    std::string m_software;
    i32 m_protocol;
    std::vector<std::string> m_features;
};


} // end ns
