/**
 * Orthanc PhotoTrack
 * Copyright (C) 2014 - Gregory Art, Jean-Francois Colson, Benjamin
 * Golinvaux, Sebastien Jodogne
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#pragma once

#include <boost/date_time.hpp>
#include <Core/RestApi/RestApiCall.h>

namespace PhotoTrack
{
  class Toolbox
  {
  public:
    static boost::posix_time::ptime Now();

    static std::string  FormatTime(const boost::posix_time::ptime& t,
                                   const std::string& format);

    static boost::posix_time::time_duration  GetUtcOffset();

    static int64_t GetSecondsSinceEpoch();

    static boost::posix_time::ptime FromTimestamp(int64_t timestamp);

    static std::string TimestampToIso8601(int64_t timestamp);

    static void ParseCookies(std::map<std::string, std::string>& cookies,
                             const std::string& header);

    static void ParseCookies(std::map<std::string, std::string>& cookies,
                             const Orthanc::RestApiCall& call);

    static bool GetSessionCookie(std::string& session,
                                 const Orthanc::RestApiCall& call);
  };
}
