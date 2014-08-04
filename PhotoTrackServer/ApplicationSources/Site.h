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

#include <Core/Uuid.h>

class Site
{
private:
  std::string  uuid_;
  std::string  pitNumber_;
  std::string  name_;
  int64_t      secondsSinceEpoch_;
  bool         hasGps_;
  float        longitude_;
  float        latitude_;
  std::string  address_;
  int          status_;

public:
  Site() :
    uuid_(Orthanc::Toolbox::GenerateUuid()),
    secondsSinceEpoch_(PhotoTrack::Toolbox::GetSecondsSinceEpoch()),
    hasGps_(false),
    longitude_(0.0f),
    latitude_(0.0f),
    status_(0)
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

  std::string GetPitNumber() const
  {
    return pitNumber_;
  }

  void SetPitNumber(std::string val)
  {
    pitNumber_ = val; 
  }

  std::string GetName() const 
  { 
    return name_; 
  }

  void SetName(std::string val) 
  { 
    name_ = val; 
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

  std::string GetAddress() const 
  { 
    return address_; 
  }

  void SetAddress(std::string val) 
  { 
    address_ = val; 
  }

  int GetStatus() const
  { 
    return status_; 
  }

  void SetStatus(int val)
  {
    status_ = val; 
  }

  void ToJson(Json::Value& value) const;

  static Site FromJson(const Json::Value& value);

  void UpdateWithJson(const Json::Value& value);
};


