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

#include "ActiveSessions.h"
#include "Database.h"

#include <Core/FileStorage/FileStorage.h>
#include <Core/RestApi/RestApi.h>
#include <set>

namespace PhotoTrack
{
  class PhotoTrackApi : public Orthanc::RestApi
  {
  private:
    ActiveSessions  sessions_;
    DatabaseWrapper* db_;

  public:
    PhotoTrackApi(bool isTest);

    void SetDatabaseWrapper(DatabaseWrapper& db)
    {
      db_ = &db;
    }

    void SetAuthenticator(IAuthenticator& authenticator)
    {
      sessions_.SetAuthenticator(authenticator);
    }

    static PhotoTrackApi& GetApi(Orthanc::RestApiCall& call)
    {
      return dynamic_cast<PhotoTrackApi&>(call.GetContext());
    }

    static ActiveSessions& GetActiveSessions(Orthanc::RestApiCall& call)
    {
      return GetApi(call).sessions_;
    }

    static DatabaseWrapper& GetDatabaseWrapper(Orthanc::RestApiCall& call);

    static Orthanc::FileStorage& GetFileStorage(Orthanc::RestApiCall& call)
    {
      return GetDatabaseWrapper(call).GetFileStorage();
    }
  };
}
