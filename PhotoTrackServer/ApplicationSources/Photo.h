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
#include "Toolbox.h"

#include "Site.h"

#include <Core/Uuid.h>

class Photo
{
  std::string   uuid_;
  std::string   imageUuid_;
  std::string   imageMime_;
  bool          hasGps_;
  float         longitude_;
  float         latitude_;
  int64_t       secondsSinceEpoch_;
  std::string   tag_;
  std::string   siteUuid_;
public:
  Photo() :
    uuid_(Orthanc::Toolbox::GenerateUuid()), imageMime_("image/jpeg"),
    hasGps_(false),
    longitude_(0.0f),
    latitude_(0.0f),
    secondsSinceEpoch_(PhotoTrack::Toolbox::GetSecondsSinceEpoch())
  {
  }

  std::string GetUuid() const
  {
    return uuid_;
  }

  void SetUuid(std::string val)
  {
    uuid_ = val;
  }

  std::string GetImageUuid() const
  {
    return imageUuid_;
  }

  void SetImageUuid(std::string val)
  {
    imageUuid_ = val;
  }

  std::string GetImageMime() const
  {
    return imageMime_;
  }

  void SetImageMime(std::string val)
  {
    imageMime_ = val;
  }

  int64_t GetSecondsSinceEpoch() const 
  { 
    return secondsSinceEpoch_; 
  }

  void SetSecondsSinceEpoch(int64_t val)
  { 
    secondsSinceEpoch_ = val; 
  }

  bool HasGps() const 
  {
    return hasGps_; 
  }

  void SetGps(float latitude, float longitude)
  {
    hasGps_ = true;
    latitude_ = latitude;
    longitude_ = longitude;
  }

  void SetNoGps()
  {
    hasGps_ = false;
    latitude_ = 0.0f;
    longitude_ = 0.0f;
  }

  float GetLongitude() const 
  { 
    return longitude_; 
  }

  float GetLatitude() const
  {
    return latitude_; 
  }

  std::string GetTag() const
  { 
    return tag_; 
  }

  void SetTag(std::string val) 
  {
    tag_ = val; 
  }

  std::string GetSiteUuid() const 
  {
    return siteUuid_; 
  }

  void SetSiteUuid(std::string val) 
  { 
    siteUuid_ = val; 
  }

  void SetSite(const Site& site)
  {
    siteUuid_ = site.GetUuid();
  }

  std::string GetTime() const;

  void ToJson(Json::Value& value) const;

  static Photo FromJson(const Json::Value& value);

  void UpdateWithJson(const Json::Value& value);
};


