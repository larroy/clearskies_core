/*
 *  This file is part of clearskies_core.

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
#include <boost/test/unit_test.hpp>
#include "cs/conf.hpp"

using namespace std;
using namespace cs::conf;

BOOST_AUTO_TEST_CASE(conf_test_01)
{
    Conf conf;
    conf.load();
    conf.daemon_port() = 1; 
    conf.save();

    Conf conf2(conf.m_db_path);
    BOOST_CHECK_EQUAL(conf2.daemon_port(), 0);
    conf2.load();
    BOOST_CHECK_EQUAL(conf2.daemon_port(), 1);
}


