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
#include "clearskiesprotocol.hpp"
#include <cassert>

using namespace std;

namespace cs
{
namespace protocol
{


void ClearSkiesProtocol::trans_INITIAL_INTERNAL_START(const message::Message& msg)
{

}

void ClearSkiesProtocol::handle_message(const message::Message& msg)
{
}

void ClearSkiesProtocol::handle_payload(const char* data, size_t len)
{
}

void ClearSkiesProtocol::handle_payload_end()
{
}

void ClearSkiesProtocol::handle_msg_garbage(const std::string& buff)
{
    assert(false);
}

void ClearSkiesProtocol::handle_pl_garbage(const std::string& buff)
{
    assert(false);
}



} // end ns
} // end ns
