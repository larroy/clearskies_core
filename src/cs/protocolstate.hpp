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
#include "message.hpp"

namespace cs
{
namespace protocol
{
/**
 * @brief Base protocol state class for all protocols
 * @author plarroy
 *
 * Input data is fed and when messages are assembled handle_message is called which implementes the
 * message dispatching logic
 */
class ProtocolState 
{
public:
    /**
     * feed input data, for example from socket IO
     * Once a full message is read, handle_message is called
     */
    virtual ~ProtocolState() = default;
    void input(const char* data, size_t len);
    virtual void handle_message(const Message&) = 0;
};

} // end ns
} // end ns
