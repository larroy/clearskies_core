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
#include "utils.hpp"
#include <array>
#include <cassert>
#include <cstdio>

using namespace std;

namespace
{

struct tm* xgmtime_r(const time_t *timep, struct tm *result)
{
    // FIXME ifdef windows here with gmtime_s (needs to be reentrant)
    return gmtime_r(timep, result);
}

}

namespace cs
{
namespace utils
{


std::string isotime(std::time_t time)
{
    auto tm_ = make_unique<tm>();
    string result(sizeof("YYYY-MM-DDThh:mm:ss.sZ"), 0);
    size_t wrote = strftime(&result[0], result.size(), "%FT%TZ", xgmtime_r(&time, tm_.get()));
    assert(wrote > 0);
    result.resize(wrote);
    return result;
}

std::string bin_to_hex(const void* b, size_t sz)
{
    const u8* p = static_cast<const u8*>(b);
    char tmp[5];
    std::string result;
    result.reserve(sz << 1);
    for (size_t i=0; i < sz; ++i)
    {
        sprintf(tmp,"%02x", p[i]);
        result += tmp;
    }
    return result;
}


} // end ns
} // end ns
