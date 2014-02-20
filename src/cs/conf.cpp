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

using namespace std;

namespace cs
{
namespace conf
{

#define conf_ctor_initializers\
    , m_daemon_port(0)

Conf::Conf(const std::string& dbpath ):
    m_tmpdir()
    , m_db_path(dbpath)
    , m_db(m_db_path.c_str())
    conf_ctor_initializers
{
    initialize_tables();
}

Conf::Conf():
    m_tmpdir(make_unique<utils::Tmpdir>())
    , m_db_path((m_tmpdir->path / "conf.db").string())
    , m_db(m_db_path.c_str())
    conf_ctor_initializers
{
    initialize_tables();
}
#undef conf_ctor_initializers


void Conf::initialize_tables()
{
    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS conf (
            dummy INTEGER,
            daemon_port INTEGER NOT NULL DEFAULT 0
        )
    )#").execute();


    // FIXME
    sqlite3pp::command(m_db, "INSERT INTO conf (dummy) VALUES (0)");

    sqlite3pp::command(m_db, R"#(CREATE TABLE IF NOT EXISTS shares (
            path TEXT PRIMARY KEY, /* path to the share */
            dbpath TEXT UNIQUE NOT NULL /* path to the share database */
        )
    )#").execute();


}

void Conf::load()
{
    sqlite3pp::query q(m_db, "SELECT daemon_port FROM conf");
    for (const auto& row: q)
    {
        m_daemon_port = row.get<int>(0);
    }
}

void Conf::save()
{
    sqlite3pp::command(m_db, R"#(UPDATE conf SET
        daemon_port = ?)#").execute();
    assert(m_db.changes() == 1);
}


} // end ns
} // end ns
