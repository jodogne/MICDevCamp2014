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

#include "Site.h"

#include <Core/OrthancException.h>

#include <boost/lexical_cast.hpp>

void Site::ToJson(Json::Value& value) const
{
  value = Json::objectValue;
  value["PitNumber"] = pitNumber_;
  value["Name"] = name_;
  value["SecondsSinceEpoch"] = boost::lexical_cast<std::string>(secondsSinceEpoch_);
  value["Time"] = PhotoTrack::Toolbox::TimestampToIso8601(secondsSinceEpoch_);
  
  if (hasGps_)
  {
    value["Longitude"] = longitude_;
    value["Latitude"] = latitude_;
  }

  value["Address"] = address_;
  value["Uuid"] = uuid_;
  value["Status"] = status_;
}

static void UpdateWithJsonInternal(Site& site, const Json::Value& value)
{
  if (value.type() != Json::objectValue)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType);
  }

  if (value.isMember("PitNumber"))
  {
    site.SetPitNumber(value["PitNumber"].asString());
  }

  if (value.isMember("Name"))
  {
    site.SetName(value["Name"].asString());
  }

  if (value.isMember("SecondsSinceEpoch"))
  {
    site.SetSecondsSinceEpoch(boost::lexical_cast<int64_t>(value["SecondsSinceEpoch"].asString()));
  }

  if (value.isMember("Longitude") && 
      value.isMember("Latitude"))
  {
    site.SetGps(value["Latitude"].asFloat(),
                value["Longitude"].asFloat());
  }

  if (value.isMember("Address"))
  {
    site.SetAddress(value["Address"].asString());
  }

  if (value.isMember("Uuid"))
  {
    site.SetUuid(value["Uuid"].asString());
  }
  
  if (value.isMember("Status"))
  {
    site.SetStatus(value["Status"].asInt());
  }
}


Site Site::FromJson(const Json::Value& value)
{
  Site site;
  site.SetNoGps();
  UpdateWithJsonInternal(site, value);
  return site;
}


void Site::UpdateWithJson(const Json::Value& value)
{
  UpdateWithJsonInternal(*this, value);
}
