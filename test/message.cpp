/*
 *  This file is part of clearskies_core.

 *  clearskies_core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  clearskies_core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with clearskies_core.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cs/message.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace cs::message;

BOOST_AUTO_TEST_CASE(MessageTest_type) {
    BOOST_CHECK(mtype_from_string("unknown") == MType::UNKNOWN);
    BOOST_CHECK(mtype_from_string("__internal_start") == MType::INTERNAL_START);
    BOOST_CHECK(mtype_from_string("ping") == MType::PING);
    BOOST_CHECK(mtype_from_string("greeting") == MType::GREETING);
    BOOST_CHECK(mtype_from_string("start") == MType::START);
    BOOST_CHECK(mtype_from_string("cannot_start") == MType::CANNOT_START);
    BOOST_CHECK(mtype_from_string("starttls") == MType::STARTTLS);
    BOOST_CHECK(mtype_from_string("identity") == MType::IDENTITY);
    BOOST_CHECK(mtype_from_string("keys") == MType::KEYS);
    BOOST_CHECK(mtype_from_string("keys_acknowledgment") == MType::KEYS_ACKNOWLEDGMENT);
    BOOST_CHECK(mtype_from_string("manifest") == MType::MANIFEST);
    BOOST_CHECK(mtype_from_string("get_manifest") == MType::GET_MANIFEST);
    BOOST_CHECK(mtype_from_string("get") == MType::GET);
    BOOST_CHECK(mtype_from_string("file_data") == MType::FILE_DATA);
    BOOST_CHECK(mtype_from_string("update") == MType::UPDATE);
    BOOST_CHECK(mtype_from_string("move") == MType::MOVE);
    BOOST_CHECK(mtype_from_string("aarsrasrasa") == MType::UNKNOWN);
}
