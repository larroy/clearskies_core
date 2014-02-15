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
#include <string>
#include "sqlite3pp/sqlite3pp.h"

namespace cs
{

namespace conf
{

/**
 * Configuration object
 */
class Conf
{
public:
    // @param db_path is the path to the database holding the configuration data
    Conf(const std::string& dbpath);

    void initialize_tables();

    sqlite3pp::database m_db;
    /// path to the sqlite database
    std::string m_db_path;
};

} // end ns
} // end ns
