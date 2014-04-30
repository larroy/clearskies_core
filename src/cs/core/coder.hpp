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
#include "message.hpp"
#include <memory>
#include <string>
#include <stdexcept>

/**
 * @file message_core_coder.hpp
 * Encoding and decoding of core messages, implementation uses the private implementation technique
 * to hide the particular message coding format and/or to support multiple encodings at runtime
 */
namespace cs
{
namespace core
{

enum class CoderType: unsigned
{
    JSON,
};

class CoderImpl;

class CoderError: public std::runtime_error
{
public:
    CoderError(const std::string& x):
        std::runtime_error(x)
    {}
};

class Coder
{
public:
    Coder(CoderType type = CoderType::JSON);
    ~Coder();

    // copies disabled
    Coder(const Coder&) = delete;
    Coder& operator=(const Coder&) = delete;

    // moves enabled
    Coder(Coder&&);
    Coder& operator=(Coder&&);

    /// @returns a pointer to a polymorphic Message
    /// @throws CoderError when the message has a format that prevents decoding
    std::unique_ptr<Message> decode_msg(bool payload, const char* encoded, size_t encoded_sz, const char* signature, size_t signature_sz);

    /// @returns a string with the encoded message
    std::string encode_msg(const Message&) const;

private:
    std::unique_ptr<CoderImpl> m_p;
};

} // end ns
} // end ns
