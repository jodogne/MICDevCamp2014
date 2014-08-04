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

#include "ServerPrecompiledHeaders.h"
#include "Toolbox.h"

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <Core/OrthancException.h>

namespace PhotoTrack
{
  boost::posix_time::ptime Toolbox::Now()
  {
    return boost::posix_time::microsec_clock::local_time();
    //return boost::posix_time::second_clock::local_time();
  }


  std::string Toolbox::FormatTime(const boost::posix_time::ptime& t,
                                  const std::string& format)
  {
    boost::posix_time::time_facet* output_facet = new boost::posix_time::time_facet;
    output_facet->format(format.c_str());
    std::ostringstream ss;
    ss.imbue(std::locale(std::locale::classic(), output_facet));
    return static_cast<std::ostringstream&>(ss << t).str();
  }


  boost::posix_time::time_duration Toolbox::GetUtcOffset() 
  {
    // http://stackoverflow.com/a/3854549/881731
    using namespace boost::posix_time;

    // boost::date_time::c_local_adjustor uses the C-API to adjust a
    // moment given in utc to the same moment in the local time zone.
    typedef boost::date_time::c_local_adjustor<ptime> local_adj;

    const ptime utc_now = second_clock::universal_time();
    const ptime now = local_adj::utc_to_local(utc_now);

    return now - utc_now;
  }


  int64_t Toolbox::GetSecondsSinceEpoch()
  {
    // http://www.boost.org/doc/libs/1_35_0/doc/html/date_time/examples.html#date_time.examples.seconds_since_epoch
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1)); 
    return (now - GetUtcOffset() - epoch).total_seconds();
  }

  
  boost::posix_time::ptime Toolbox::FromTimestamp(int64_t timestamp)
  {
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1)); 
    return epoch + boost::posix_time::seconds(static_cast<long>(timestamp));
  }
    
  std::string Toolbox::TimestampToIso8601(int64_t timestamp)
  {
    return boost::posix_time::to_iso_string(PhotoTrack::Toolbox::FromTimestamp(timestamp)) + "Z";
  }


  void Toolbox::ParseCookies(std::map<std::string, std::string>& cookies,
                             const std::string& header)
  {
    cookies.clear();

    std::vector<std::string> tokens;
    Orthanc::Toolbox::TokenizeString(tokens, header, ';');

    for (size_t i = 0; i < tokens.size(); i++)
    {
      if (tokens[i].empty())
      {
        continue;
      }

      std::vector<std::string> cookie;
      Orthanc::Toolbox::TokenizeString(cookie, tokens[i], '=');

      if (cookie.size() != 2)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      std::string name = Orthanc::Toolbox::StripSpaces(cookie[0]);
      std::string value = Orthanc::Toolbox::StripSpaces(cookie[1]);
      cookies[name] = value;
    }
  }


  void Toolbox::ParseCookies(std::map<std::string, std::string>& cookies,
                             const Orthanc::RestApiCall& call)
  {
    ParseCookies(cookies, call.GetHttpHeader("cookie", ""));
  }


  bool Toolbox::GetSessionCookie(std::string& session,
                                 const Orthanc::RestApiCall& call)
  {
    typedef std::map<std::string, std::string> Cookies;

    Cookies cookies;
    ParseCookies(cookies, call);
    
    Cookies::const_iterator it = cookies.find("session");
    if (it == cookies.end())
    {
      return false;
    }
    else
    {
      session = it->second;
      return true;
    }
  }

}
