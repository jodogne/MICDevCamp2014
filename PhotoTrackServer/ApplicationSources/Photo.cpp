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

#include "Photo.h"

#include <Core/OrthancException.h>


void Photo::ToJson(Json::Value& value) const
{
  value = Json::objectValue;
  value["SecondsSinceEpoch"] = boost::lexical_cast<std::string>(secondsSinceEpoch_);
  value["Time"] = GetTime();
  value["ImageMime"] = imageMime_;
  value["SiteUuid"] = siteUuid_;

  if (hasGps_)
  {
    value["Longitude"] = longitude_;
    value["Latitude"] = latitude_;
  }

  value["Tag"] = tag_;
  value["Uuid"] = uuid_;
}

static void UpdateWithJsonInternal(Photo& photo, const Json::Value& value)
{
  if (value.type() != Json::objectValue)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType);
  }

  if (value.isMember("SecondsSinceEpoch"))
  {
    photo.SetSecondsSinceEpoch(boost::lexical_cast<int64_t>(value["SecondsSinceEpoch"].asString()));
  }

  if (value.isMember("Longitude") && value.isMember("Latitude"))
  {
    photo.SetGps(value["Latitude"].asFloat(), value["Longitude"].asFloat());
  }

  if (value.isMember("Uuid"))
  {
    photo.SetUuid(value["Uuid"].asString());
  }

  if (value.isMember("SiteUuid"))
  {
    photo.SetSiteUuid(value["SiteUuid"].asString());
  }

  if (value.isMember("Tag"))
  {
    photo.SetTag(value["Tag"].asString());
  }
}

void Photo::UpdateWithJson(const Json::Value& value)
{
  UpdateWithJsonInternal(*this, value);
}

Photo Photo::FromJson(const Json::Value& value)
{
  Photo photo;
  photo.SetNoGps();
  UpdateWithJsonInternal(photo, value);
  return photo;
}

std::string Photo::GetTime() const
{
  return PhotoTrack::Toolbox::TimestampToIso8601(secondsSinceEpoch_);
}


