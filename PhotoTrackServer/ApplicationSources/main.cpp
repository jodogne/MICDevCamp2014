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

#include "EmbeddedResources.h"
#include "PhotoTrackApi.h"
#include "PropertyMap.h"
#include "Configuration.h"
#include "Toolbox.h"
#include "Database.h"

#include <Core/FileStorage/FileStorage.h>
#include <Core/HttpServer/MongooseServer.h>
#include <Core/HttpServer/FilesystemHttpHandler.h>
#include <Core/Toolbox.h>
#include <Core/SQLite/Connection.h>
#include <Core/SQLite/Statement.h>
#include <Core/Uuid.h>

#include <memory>
#include <stdio.h>
#include <glog/logging.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



namespace
{
  class DummyAuthenticator : public PhotoTrack::IAuthenticator
  {
  public:
    virtual bool Authenticate(const std::string& username,
      const std::string& password)
    {
      return (username == "user" &&
        password == "pass");
    }

    virtual PhotoTrack::IClonable* GetUserPayload(const std::string& username)
    {
      std::auto_ptr<PhotoTrack::PropertyMap> p(new PhotoTrack::PropertyMap);

      p->SetValue("username", username);

      return p.release();
    }
  };
}


static void PrintHelp(char* path)
{
  // TODO
}


static void PrintVersion(char* path)
{
  // TODO
}






int main(int argc, char* argv[])
{
  // Initialize Google's logging library.
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 1;
  FLAGS_v = 0;

  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "--help")
    {
      PrintHelp(argv[0]);
      return 0;
    }

    if (std::string(argv[i]) == "--version")
    {
      PrintVersion(argv[0]);
      return 0;
    }

    if (std::string(argv[i]) == "--verbose")
    {
      FLAGS_minloglevel = 0;
    }

    if (std::string(argv[i]) == "--trace")
    {
      FLAGS_minloglevel = 0;
      FLAGS_v = 1;
    }

    if (boost::starts_with(argv[i], "--logdir="))
    {
      FLAGS_logtostderr = false;
      FLAGS_log_dir = std::string(argv[i]).substr(9);
    }
  }

  google::InitGoogleLogging("PhotoTrack");


  bool isInitialized = false;
  for (int i = 1; i < argc; i++)
  {
    // Use the first argument that does not start with a "-" as
    // the configuration file
    if (argv[i][0] != '-')
    {
      PhotoTrack::Configuration::Initialize(argv[i]);
      isInitialized = true;
    }
  }

  if (!isInitialized)
  {
    PhotoTrack::Configuration::Initialize(NULL);
  }


  LOG(WARNING) << PhotoTrack::Configuration::GetPath("Assets", "test.cpp");

  Orthanc::FileStorage storage(PhotoTrack::Configuration::GetPath("FileStorage", "FileStorage"));
  DatabaseWrapper database(PhotoTrack::Configuration::GetPath("Database", "index.db"), storage);

  {
    DummyAuthenticator authenticator;

    PhotoTrack::PhotoTrackApi api(true /* TEST: TODO */);
    api.SetAuthenticator(authenticator);
    api.SetDatabaseWrapper(database);

    Orthanc::MongooseServer httpServer;
    httpServer.SetRemoteAccessAllowed(true);   // TODO : For security
    httpServer.RegisterHandler(api);
    httpServer.SetPortNumber(PhotoTrack::Configuration::GetInteger("HttpPort", 8000));

    std::auto_ptr<Orthanc::FilesystemHttpHandler> assets;
    if (PhotoTrack::Configuration::HasParameter("Assets"))
    {
      std::string path = PhotoTrack::Configuration::GetPath("Assets", "Assets");
      LOG(WARNING) << "Serving assets from path " << path;
      assets.reset(new Orthanc::FilesystemHttpHandler("/assets", path));
      httpServer.RegisterHandler(*assets);
    }

    httpServer.Start();

    LOG(WARNING) << "HTTP server listening on port: " << httpServer.GetPortNumber();
    LOG(WARNING) << "PhotoTrack has started";
    Orthanc::Toolbox::ServerBarrier();
    LOG(WARNING) << "PhotoTrack is stopping";
  }

  LOG(WARNING) << "PhotoTrack has stopped";

  PhotoTrack::Configuration::Finalize();

  return 0;
}
