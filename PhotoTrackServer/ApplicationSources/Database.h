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

#include "Site.h"
#include "User.h"
#include "Photo.h"

#include <Core/SQLite/Connection.h>
#include <Core/FileStorage/FileStorage.h>
#include <boost/thread.hpp>

enum ChangeType
{
  ChangeType_NewImage
};

class DatabaseWrapper : public boost::noncopyable
{
private:
  boost::recursive_mutex mutex_;
  Orthanc::SQLite::Connection db_;
  Orthanc::FileStorage& fileStorage_;

  void GetChangesInternal(Json::Value& target,
                          Orthanc::SQLite::Statement& s,
                          int64_t since,
                          unsigned int maxResults);

public:
  DatabaseWrapper(const std::string& path,
                  Orthanc::FileStorage& fileStorage);

  Orthanc::FileStorage& GetFileStorage() const
  {
    return fileStorage_;
  }

  void CreateOrUpdateSite(const Site& site);
  void CreateOrUpdateUser(const User& user);
  void CreateOrUpdatePhoto(const Photo& photo);

  bool GetPhoto(Photo& photo,
                const std::string& uuid);
  bool GetUser(User& user, const std::string& uuid);
  bool GetSite(Site& site, const std::string& uuid);

  void GetSites(Json::Value& sites);
  void GetUsers(Json::Value& users);
  void GetPhotos(Json::Value& photos);
  void GetPhotos(Json::Value& photos,
                 const std::string& siteUuid);
  void GetPhotos(Json::Value& photos,
                 const Site& site);

  void DeleteSite(const std::string& uuid);
  void DeletePhoto(const std::string& uuid);
  void DeleteUser(const std::string& uuid);

  void ReplaceImage(const std::string& photoUuid,
                    const std::string& image,
                    const std::string& mimeType);

  void GetChanges(Json::Value& target,
                  int64_t since,
                  unsigned int maxResults);

  void GetLastChange(Json::Value& target);
};
