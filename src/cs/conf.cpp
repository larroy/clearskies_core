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

#include "conf.hpp"
namespace cs
{
namespace conf
{

Conf::Conf(const std::string& dbpath ):
    m_db(dbpath.c_str())
    , m_db_path(dbpath)
{
    initialize_tables();
}

void Conf::initialize_tables()
{
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS conf (
        )
    )#").execute();


    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS shares (
            path TEXT PRIMARY KEY, /* path to the share */
            dbpath TEXT UNIQUE NOT NULL /* path to the share database */
        )
    )#").execute();


}

} // end ns
} // end ns
