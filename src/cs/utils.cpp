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
#include <cstring>
#include "boost/format.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;

namespace
{

struct tm* xgmtime_r(const time_t *timep, struct tm *result)
{
    // FIXME ifdef windows here with gmtime_s (needs to be reentrant)
#ifdef CS_PLATFORM_UNIX
    return gmtime_r(timep, result);
#else
    #error This platform is not supported yet
#endif
}

}

namespace cs
{
namespace utils
{


namespace {

int64_t ptime_to_64(const boost::posix_time::ptime& pt)
{
  using namespace boost::posix_time;
  static ptime epoch(boost::gregorian::date(1970, 1, 1));
  time_duration diff(pt - epoch);
  return (diff.ticks() / diff.ticks_per_second());
}

}

std::string isotime(std::time_t time)
{
#if 0
    auto tm_ = make_unique<tm>();
    string result(sizeof("YYYY-MM-DDThh:mm:ss.sZ"), 0);
    size_t wrote = strftime(&result[0], result.size(), "%FT%TZ", xgmtime_r(&time, tm_.get()));
    assert(wrote > 0);
    result.resize(wrote);
    return result;
#endif
    using namespace boost::posix_time;
    return to_iso_string(from_time_t(time)) + "Z";
}

std::time_t isotime_from_str(const std::string& stime)
{
#if 0
    tm tm_;
    bzero(&tm_, sizeof(tm));
    const char* s = stime.c_str();
    strptime(s, "%FT%TZ", &tm_);
    return mktime(&tm_);
    // it's one hour off... 
#endif
    using namespace boost::posix_time;
    string s = stime;
    if (! s.empty())
    {
        s.erase(s.size() - 1);
        return ptime_to_64(from_iso_string(stime));
    }
    else
        throw std::runtime_error("Bad time string: " + stime);
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


std::string random_bytes(size_t count)
{
    string result;
    random_device rng;
    uniform_int_distribution<u8> udist8;
    generate_n(back_inserter(result), count, [&] { return udist8(rng); });
    return result;
}


std::string read_file(const bfs::path& path)
{
    string result;
    const size_t BSIZE = 8192;
    std::array<char, BSIZE > rbuff;
    bfs::ifstream ifs(path);
    if (! ifs)
        throw std::runtime_error(boost::str(boost::format("utils::read_file %1%") % path.string()));

    ifs.exceptions(ios::badbit);
    do
    {
        ifs.read(rbuff.data(), rbuff.size());
        result.append(rbuff.data(), ifs.gcount());
    }
    while (ifs);

    return result;
}


} // end ns
} // end ns
