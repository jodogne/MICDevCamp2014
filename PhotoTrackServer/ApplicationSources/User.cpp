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

#include "User.h"

#include <Core/OrthancException.h>

void User::ToJson(Json::Value& value) const
{
  value = Json::objectValue;
  
  // std::string password_;
  value["Password"] = password_;

  //std::string userName_;
  value["UserName"] = userName_;
  
  //std::string fullName_;
  value["FullName"] = fullName_;

  //std::string email_;
  value["Email"] = email_;
  
  //bool isSupervisor_;
  value["IsSupervisor"] = isSupervisor_;

  //bool isAdmin_;
  value["IsAdmin"] = isAdmin_;

  //std::string organization_;
  value["Organization"] = organization_;

  //std::string organization_;
  value["Uuid"] = uuid_;
}

static void UpdateWithJsonInternal(User& user, const Json::Value& value)
{
  if (value.type() != Json::objectValue)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType);
  }

  if (value.isMember("Password"))
  {
    user.SetPassword(value["Password"].asString());
  }

  if (value.isMember("UserName"))
  {
    user.SetUserName(value["UserName"].asString());
  }

  if (value.isMember("FullName"))
  {
    user.SetFullName(value["FullName"].asString());
  }

  if (value.isMember("Email"))
  {
    user.SetEmail(value["Email"].asString());
  }

  if (value.isMember("IsSupervisor"))
  {
    user.SetIsSupervisor(value["IsSupervisor"].asBool());
  }

  if (value.isMember("IsAdmin"))
  {
    user.SetIsAdmin(value["IsAdmin"].asBool());
  }

  if (value.isMember("Organization"))
  {
    user.SetOrganization(value["Organization"].asString());
  }

  if (value.isMember("Uuid"))
  {
    user.SetUuid(value["Uuid"].asString());
  }
}

User User::FromJson(const Json::Value& value)
{
  User user;
  UpdateWithJsonInternal(user, value);
  return user;
}

void User::UpdateWithJson(const Json::Value& value)
{
  UpdateWithJsonInternal(*this, value);
}
