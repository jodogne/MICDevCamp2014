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

#include <string>
#include <json/value.h>
#include <Core/Uuid.h>

class User
{
public:
  User() :
    uuid_(Orthanc::Toolbox::GenerateUuid()),
    isSupervisor_(false),
    isAdmin_(false)
  {}

private:
  std::string uuid_;
  std::string password_;
  std::string userName_;
  std::string fullName_;
  std::string email_;
  bool isSupervisor_;
  bool isAdmin_;
  std::string organization_;

public:
  std::string GetUuid() const
  {
    return uuid_;
  }

  void        SetUuid(std::string val)
  {
    uuid_ = val;
  }

  std::string GetUserName() const
  {
    return userName_;
  }

  void SetUserName(std::string val)
  {
    userName_ = val;
  }

  std::string GetPassword() const
  { 
    return password_;
  }

  void SetPassword(std::string val)
  {
    password_ = val; 
  }

  std::string GetFullName() const 
  { 
    return fullName_;
  }

  void SetFullName(std::string val)
  { 
    fullName_ = val; 
  }

  std::string GetEmail() const 
  { 
    return email_; 
  }

  void SetEmail(std::string val) 
  {
    email_ = val; 
  }

  bool IsSupervisor() const 
  {
    return isSupervisor_; 
  }

  void SetIsSupervisor(bool val)
  {
    isSupervisor_ = val; 
  }

  bool IsAdmin() const 
  { 
    return isAdmin_; 
  }

  void SetIsAdmin(bool val)
  { 
    isAdmin_ = val; 
  }

  std::string GetOrganization() const 
  {
    return organization_; 
  }

  void SetOrganization(std::string val)
  { 
    organization_ = val; 
  }

  void ToJson(Json::Value& value) const;

  static User FromJson(const Json::Value& value);

  void UpdateWithJson(const Json::Value& value);
};



