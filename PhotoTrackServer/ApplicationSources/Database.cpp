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

#include "Database.h"

#include "Toolbox.h"
#include "EmbeddedResources.h"
#include "Site.h"
#include "User.h"
#include "Photo.h"

#include <Core/Toolbox.h>
#include <Core/Uuid.h>
#include <Core/SQLite/Connection.h>
#include <Core/SQLite/Statement.h>

#include <glog/logging.h>
#include <boost/thread.hpp>

namespace
{
  class SignalImageDeleted : public Orthanc::SQLite::IScalarFunction
  {
  private:
    Orthanc::FileStorage& fileStorage_;
    
  public:
    SignalImageDeleted(Orthanc::FileStorage& fileStorage) : 
      fileStorage_(fileStorage)
    {
    }
    
    virtual const char* GetName() const
    {
      return "SignalImageDeleted";
    }
    
    virtual unsigned int GetCardinality() const
    {
      return 1;
    }

    virtual void Compute(Orthanc::SQLite::FunctionContext& context)
    {
      std::string uuid = context.GetStringValue(0);

      if (!uuid.empty())
      {
        LOG(INFO) << "Remove stored image with UUID " << uuid;
        fileStorage_.Remove(uuid);
      }
    }
  };
}


DatabaseWrapper::DatabaseWrapper(const std::string& path,
                                 Orthanc::FileStorage& fileStorage) :
  fileStorage_(fileStorage)
{
  LOG(WARNING) << "Using the following SQLite database: " << path;

  bool createDatabase = !Orthanc::Toolbox::IsExistingFile(path);

  db_.Open(path);

  if (createDatabase)
  {
    LOG(WARNING) << "Creating the database";

    // Performance tuning of SQLite with PRAGMAs
    // http://www.sqlite.org/pragma.html
    db_.Execute("PRAGMA SYNCHRONOUS=NORMAL;");
    db_.Execute("PRAGMA JOURNAL_MODE=WAL;");
    db_.Execute("PRAGMA LOCKING_MODE=EXCLUSIVE;");
    db_.Execute("PRAGMA WAL_AUTOCHECKPOINT=1000;");
    //db_.Execute("PRAGMA TEMP_STORE=memory");

    std::string s;
    //Orthanc::Toolbox::ReadFile(s, "PrepareDatabase.sql");
    Orthanc::EmbeddedResources::GetFileResource(s, Orthanc::EmbeddedResources::PREPARE_DATABASE);
    db_.Execute(s);
  }

  db_.Register(new SignalImageDeleted(fileStorage_));
}

/*
  CREATE TABLE Sites(
  0 uuid TEXT PRIMARY KEY,
  1 pitNumber TEXT,
  2 name TEXT,
  3 secondsSinceEpoch INTEGER,
  4 longitude REAL,
  5 latitude REAL,
  6 address TEXT,
  7 status INTEGER
  );
*/

void DatabaseWrapper::CreateOrUpdateSite(const Site& site)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "INSERT OR REPLACE INTO Sites VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
  s.BindString(0, site.GetUuid());
  s.BindString(1, site.GetPitNumber());
  s.BindString(2, site.GetName());
  s.BindInt64(3, site.GetSecondsSinceEpoch());

  if (site.HasGps())
  {
    s.BindDouble(4, site.GetLatitude());
    s.BindDouble(5, site.GetLongitude());
  }
  else
  {
    s.BindNull(4);
    s.BindNull(5);
  }

  s.BindString(6, site.GetAddress());
  s.BindInt(7, site.GetStatus());
  s.Run();
}

/*
  CREATE TABLE Users(
  0 uuid TEXT PRIMARY KEY,
  1 username TEXT,
  2 password TEXT,   -- Hashed
  3 fullName TEXT,
  4 email TEXT,

  5 isSupervisor INT,
  6 isAdmin INT,

  7 organization TEXT
  );
*/
void DatabaseWrapper::CreateOrUpdateUser(const User& user)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "INSERT OR REPLACE INTO Users VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
  s.BindString(0, user.GetUuid());
  s.BindString(1, user.GetUserName());
  s.BindString(2, user.GetPassword());
  s.BindString(3, user.GetFullName());
  s.BindString(4, user.GetEmail());

  s.BindInt(5, user.IsSupervisor());
  s.BindInt(6, user.IsAdmin());

  s.BindString(7, user.GetOrganization());
  s.Run();
}

/*
  CREATE TABLE Photos(
  0 uuid TEXT PRIMARY KEY,
  1 imageUuid TEXT,
  2 longitude REAL,
  3 latitude REAL,
  4 secondsSinceEpoch INTEGER,
  5 tag TEXT,
  6 siteUuid TEXT REFERENCES Sites(uuid) ON DELETE CASCADE
  );
*/

void DatabaseWrapper::CreateOrUpdatePhoto(const Photo& photo)
{
  if (photo.GetSiteUuid().empty())
  {
    LOG(ERROR) << "Creating a photo without a parent site";
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem);
  }

  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "INSERT OR REPLACE INTO Photos VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
  s.BindString(0, photo.GetUuid());
  s.BindString(1, photo.GetImageUuid());
  s.BindString(2, photo.GetImageMime());

  if (photo.HasGps())
  {
    s.BindDouble(3, photo.GetLatitude());
    s.BindDouble(4, photo.GetLongitude());
  }
  else
  {
    s.BindNull(3);
    s.BindNull(4);
  }

  s.BindInt64(5, photo.GetSecondsSinceEpoch());
  s.BindString(6, photo.GetTag());
  s.BindString(7, photo.GetSiteUuid());
  s.Run();
}


bool DatabaseWrapper::GetPhoto(Photo& photo,
                               const std::string& uuid)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT * FROM Photos WHERE uuid=?");
  s.BindString(0, uuid);

  if (!s.Step())
  { 
    return false;
  }
  else
  {
    photo.SetUuid(s.ColumnString(0));
    photo.SetImageUuid(s.ColumnString(1));
    photo.SetImageMime(s.ColumnString(2));
      
    if (s.ColumnIsNull(3))
    {
      photo.SetNoGps();
    }
    else
    {
      photo.SetGps(static_cast<float>(s.ColumnDouble(3)),
                   static_cast<float>(s.ColumnDouble(4)));
    }

    photo.SetSecondsSinceEpoch(s.ColumnInt64(5));
    photo.SetTag(s.ColumnString(6));
    photo.SetSiteUuid(s.ColumnString(7));
      
    return true;
  }    
}

bool DatabaseWrapper::GetUser(User& user, const std::string& uuid)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT * FROM Users WHERE uuid=?");
  s.BindString(0, uuid);

  if (!s.Step())
  {
    return false;
  }
  else
  {
    user.SetUuid(s.ColumnString(0));
    user.SetUserName(s.ColumnString(1));
    user.SetPassword(s.ColumnString(2));
    user.SetFullName(s.ColumnString(3));
    user.SetEmail(s.ColumnString(4));

    user.SetIsSupervisor(s.ColumnInt(5) != 0);
    user.SetIsAdmin(s.ColumnInt(6) != 0);
    user.SetOrganization(s.ColumnString(7));
    return true;
  }
}


bool DatabaseWrapper::GetSite(Site& site, const std::string& uuid)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT * FROM Sites WHERE uuid=?");
  s.BindString(0, uuid);

  if (!s.Step())
  {
    return false;
  }
  else
  {
    site.SetUuid(s.ColumnString(0));
    site.SetPitNumber(s.ColumnString(1));
    site.SetName(s.ColumnString(2));
    site.SetSecondsSinceEpoch(s.ColumnInt64(3));
      
    if (s.ColumnIsNull(4))
    {
      site.SetNoGps();
    }
    else
    {
      site.SetGps(static_cast<float>(s.ColumnDouble(4)),static_cast<float>(s.ColumnDouble(5)));
    }
      
    site.SetAddress(s.ColumnString(6));
    site.SetStatus(s.ColumnInt(7));
    return true;
  }
}

void DatabaseWrapper::GetSites(Json::Value& sites)
{
  sites = Json::arrayValue;

  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT uuid FROM Sites");

  while (s.Step())
  {
    Site site;
    GetSite(site, s.ColumnString(0));
    Json::Value jsonSite;
    site.ToJson(jsonSite);
    sites.append(jsonSite);
  }
}


void DatabaseWrapper::GetUsers(Json::Value& users)
{
  users = Json::arrayValue;

  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT uuid FROM Users");

  while (s.Step())
  {
    User user;
    GetUser(user, s.ColumnString(0));
    Json::Value jsonUser;
    user.ToJson(jsonUser);
    users.append(jsonUser);
  }
}

void DatabaseWrapper::GetPhotos(Json::Value& photos)
{
  photos = Json::arrayValue;

  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT uuid FROM Photos");

  while (s.Step())
  {
    Photo photo;
    GetPhoto(photo, s.ColumnString(0));
    Json::Value jsonPhoto;
    photo.ToJson(jsonPhoto);
    photos.append(jsonPhoto);
  }
}

void DatabaseWrapper::GetPhotos(Json::Value& photos, const std::string& siteUuid)
{
  photos = Json::arrayValue;

  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT uuid FROM Photos WHERE siteUuid=?");
  s.BindString(0, siteUuid);

  while (s.Step())
  {
    Photo photo;
    GetPhoto(photo, s.ColumnString(0));
    Json::Value jsonPhoto;
    photo.ToJson(jsonPhoto);
    photos.append(jsonPhoto);
  }
}

void DatabaseWrapper::GetPhotos(Json::Value& photos, const Site& site)
{
  GetPhotos(photos, site.GetUuid());
}


void DatabaseWrapper::DeleteSite(const std::string& uuid)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "DELETE FROM Sites WHERE uuid=?");
  s.BindString(0, uuid);
  s.Run();
}


void DatabaseWrapper::DeletePhoto(const std::string& uuid)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "DELETE FROM Photos WHERE uuid=?");
  s.BindString(0, uuid);
  s.Run();
}


void DatabaseWrapper::DeleteUser(const std::string& uuid)
{
  using namespace Orthanc;
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  SQLite::Statement s(db_, SQLITE_FROM_HERE, "DELETE FROM Users WHERE uuid=?");
  s.BindString(0, uuid);
  s.Run();
}


void DatabaseWrapper::ReplaceImage(const std::string& photoUuid,
                                   const std::string& image,
                                   const std::string& mimeType)
{
  boost::unique_lock<boost::recursive_mutex> lock(mutex_);

  Photo photo;
  if (GetPhoto(photo, photoUuid))
  {
    std::string imageUuid = fileStorage_.Create(image);

    if (!photo.GetImageUuid().empty())
    {
      fileStorage_.Remove(photo.GetImageUuid());
    }

    photo.SetImageUuid(imageUuid);
    photo.SetImageMime(mimeType);
    CreateOrUpdatePhoto(photo);

    using namespace Orthanc;
    SQLite::Statement s(db_, SQLITE_FROM_HERE, "INSERT INTO Changes VALUES(NULL, ?, ?, ?)");
    s.BindInt(0, ChangeType_NewImage);
    s.BindString(1, photo.GetUuid());
    s.BindString(2, PhotoTrack::Toolbox::TimestampToIso8601(PhotoTrack::Toolbox::GetSecondsSinceEpoch()));
    s.Run();      
  }
  else
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem);
  }
}

static const char* EnumerationToString(ChangeType change)
{
  switch (change)
  {
    case ChangeType_NewImage:
      return "NewImage";

    default:
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
  }
}

void DatabaseWrapper::GetChangesInternal(Json::Value& target,
                                         Orthanc::SQLite::Statement& s,
                                         int64_t since,
                                         unsigned int maxResults)
{
  Json::Value changes = Json::arrayValue;
  int64_t last = since;

  while (changes.size() < maxResults && s.Step())
  {
    int64_t seq = s.ColumnInt64(0);
    ChangeType changeType = static_cast<ChangeType>(s.ColumnInt(1));
    const std::string& photoUuid = s.ColumnString(2);
    const std::string& date = s.ColumnString(3);

    Json::Value item = Json::objectValue;
    item["Seq"] = static_cast<int>(seq);
    item["ChangeType"] = EnumerationToString(changeType);
    item["PhotoUuid"] = photoUuid;
    item["Date"] = date;
    last = seq;

    changes.append(item);
  }

  target = Json::objectValue;
  target["Changes"] = changes;
  target["Done"] = !(changes.size() == maxResults && s.Step());
  target["Last"] = static_cast<int>(last);
}


void DatabaseWrapper::GetChanges(Json::Value& target,
                                 int64_t since,
                                 unsigned int maxResults)
{
  using namespace Orthanc;
  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT * FROM Changes WHERE seq>? ORDER BY seq LIMIT ?");
  s.BindInt64(0, since);
  s.BindInt(1, maxResults + 1);
  GetChangesInternal(target, s, since, maxResults);
}


void DatabaseWrapper::GetLastChange(Json::Value& target)
{
  using namespace Orthanc;
  SQLite::Statement s(db_, SQLITE_FROM_HERE, "SELECT * FROM Changes ORDER BY seq DESC LIMIT 1");
  GetChangesInternal(target, s, 0, 1);
}
