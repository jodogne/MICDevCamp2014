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
#include "PhotoTrackApi.h"

#include "PropertyMap.h"

#include <Core/Uuid.h>
#include <Core/Compression/HierarchicalZipWriter.h>
#include <Core/HttpServer/FilesystemHttpSender.h>

#include <glog/logging.h>
#include <iostream>

namespace PhotoTrack
{
  static void AnswerListAsJson(Orthanc::RestApiOutput& output,
                               const std::list<std::string> source)
  {
    Json::Value result = Json::arrayValue;

    for (std::list<std::string>::const_iterator
           it = source.begin(); it != source.end(); it++)
    {
      result.append(*it);
    }

    output.AnswerJson(result);
  }

  static void GetSessions(Orthanc::RestApiGetCall& call)
  {
    std::list<std::string> lst;
    PhotoTrackApi::GetActiveSessions(call).ListSessions(lst);

    AnswerListAsJson(call.GetOutput(), lst);
  }

  static void PostSession(Orthanc::RestApiPostCall& call)
  {
    ActiveSessions& sessions = PhotoTrackApi::GetActiveSessions(call);

    Json::Value query;
    if (call.ParseJsonRequest(query) &&
        query.isMember("username") &&
        query.isMember("password"))
    {
      std::string sessionId;
      if (sessions.OpenSession(sessionId, 
                               query["username"].asString(),
                               query["password"].asString()))
      {
        call.GetOutput().GetLowLevelOutput().SetCookie
          ("session", sessions.FormatCookie(sessionId));

        Json::Value v = Json::objectValue;
        v["SessionId"] = sessionId;
        call.GetOutput().AnswerJson(v);
      }
      else
      {
        call.GetOutput().SignalError(Orthanc::HttpStatus_401_Unauthorized);
      }
    }
  }

  static void DeleteSession(Orthanc::RestApiDeleteCall& call)
  {
    ActiveSessions& sessions = PhotoTrackApi::GetActiveSessions(call);

    std::string sessionId = call.GetUriComponent("id", "");
    sessions.CloseSession(sessionId);

    call.GetOutput().GetLowLevelOutput().SetCookie
      ("session", sessions.FormatResetCookie());
    call.GetOutput().AnswerBuffer("{}", "application/json");
  }

  static void GetSessionProperties(Orthanc::RestApiGetCall& call)
  {
    std::string sessionId = call.GetUriComponent("id", "");

    std::auto_ptr<IClonable> payload(PhotoTrackApi::GetActiveSessions(call).GetPayload(sessionId));
    if (payload.get() != NULL)
    {
      PropertyMap& m = dynamic_cast<PropertyMap&>(*payload);

      std::list<std::string> lst;
      m.ListProperties(lst);

      AnswerListAsJson(call.GetOutput(), lst);
    }
  }

  static void GetSessionPropertyValue(Orthanc::RestApiGetCall& call)
  {
    std::string sessionId = call.GetUriComponent("id", "");
    std::string property = call.GetUriComponent("property", "");

    std::auto_ptr<IClonable> payload(PhotoTrackApi::GetActiveSessions(call).GetPayload(sessionId));
    if (payload.get() != NULL)
    {
      PropertyMap& m = dynamic_cast<PropertyMap&>(*payload);
      call.GetOutput().AnswerBuffer(m.GetValue(property), "text/plain");
    }
  }

  static void RedirectToAssets(Orthanc::RestApiGetCall& call)
  {
    call.GetOutput().Redirect("assets/index.html");
  }


  DatabaseWrapper& PhotoTrackApi::GetDatabaseWrapper(Orthanc::RestApiCall& call)
  {
    DatabaseWrapper* db = GetApi(call).db_;

    if (db == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    return *db;
  }
  
  //Register("/users", ListUsers);
  static void ListUsers(Orthanc::RestApiGetCall& call)
  {
    Json::Value lst;
    PhotoTrackApi::GetDatabaseWrapper(call).GetUsers(lst);
    

    call.GetOutput().AnswerJson(lst);
  }

  //Register("/users/{uuid}/full-name", GetUserUserName);
  static void GetUserUserName(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
        call.GetOutput().AnswerBuffer(user.GetUserName(), "text/plain");
    }
  }
  
  //Register("/users/{uuid}/password", GetUserPassword);
  static void GetUserPassword(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      call.GetOutput().AnswerBuffer(user.GetPassword(), "text/plain");
    }
  }

  //Register("/users/{uuid}/full-name", GetUserFullName);
  static void GetUserFullName(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      call.GetOutput().AnswerBuffer(user.GetFullName(), "text/plain");
    }
  }

  //Register("/users/{uuid}/email", GetUserEmail);
  static void GetUserEmail(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      call.GetOutput().AnswerBuffer(user.GetEmail(), "text/plain");
    }
  }

  //Register("/users/{uuid}/is-supervisor", GetUserIsSupervisor);
  static void GetUserIsSupervisor(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      call.GetOutput().AnswerBuffer(user.IsSupervisor() ? "1" : "0", "text/plain");
    }
  }

  //Register("/users/{uuid}/is-admin", GetUserIsAdmin);
  static void GetUserIsAdmin(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      call.GetOutput().AnswerBuffer(user.IsAdmin() ? "1" : "0", "text/plain");
    }
  }

  //Register("/users/{uuid}/organization", GetUserOrganization);
  static void GetUserOrganization(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      call.GetOutput().AnswerBuffer(user.GetOrganization(), "text/plain");
    }
  }

  static void ListSites(Orthanc::RestApiGetCall& call)
  {
    /*
      std::string session;
    if (PhotoTrack::Toolbox::GetSessionCookie(session, call))    
      LOG(WARNING) << "Session cookie: " << session;
    else
      LOG(WARNING) << "No session cookie!";
    */

    Json::Value lst;
    PhotoTrackApi::GetDatabaseWrapper(call).GetSites(lst);
    call.GetOutput().AnswerJson(lst);
  }

  static void ListPhotos(Orthanc::RestApiGetCall& call)
  {
    Json::Value lst;
    PhotoTrackApi::GetDatabaseWrapper(call).GetPhotos(lst);
    call.GetOutput().AnswerJson(lst);
  }

  static void ListPhotosOfSite(Orthanc::RestApiGetCall& call)
  {
    std::string siteUuid = call.GetUriComponent("uuid", "");
    
    Json::Value lst;
    PhotoTrackApi::GetDatabaseWrapper(call).GetPhotos(lst, siteUuid);
    call.GetOutput().AnswerJson(lst);
  }

  static void GetSite(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      Json::Value v;
      site.ToJson(v);
      call.GetOutput().AnswerJson(v);
    }
  }

  static void GetPhoto(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Photo photo;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid))
    {
      Json::Value v;
      photo.ToJson(v);
      call.GetOutput().AnswerJson(v);
    }
  }

  static void GetUser(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    User user;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid))
    {
      Json::Value v;
      user.ToJson(v);
      call.GetOutput().AnswerJson(v);
    }
  }

  static void GetSiteLatitude(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      if (site.HasGps())
        call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(site.GetLatitude()), "text/plain");
    }
  }

  static void GetSiteLongitude(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      if (site.HasGps())
        call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(site.GetLongitude()), "text/plain");
    }
  }

  static void GetSitePitNumber(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      call.GetOutput().AnswerBuffer(site.GetPitNumber(), "text/plain");
    }
  }

  static void GetSiteSecondsSinceEpoch(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(site.GetSecondsSinceEpoch()), "text/plain");
    }
  }

  static void GetSiteAddress(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      call.GetOutput().AnswerBuffer(site.GetAddress(), "text/plain");
    }
  }

  static void GetSiteName(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      call.GetOutput().AnswerBuffer(site.GetName(), "text/plain");
    }
  }

  static void GetSiteStatus(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Site site;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
    {
      call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(site.GetStatus()), "text/plain");
    }
  }

  static void GetPhotoLatitude(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Photo photo;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid))
    {
      if (photo.HasGps())
        call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(photo.GetLatitude()), "text/plain");
    }
  }

  static void GetPhotoLongitude(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Photo photo;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid))
    {
      if (photo.HasGps())
        call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(photo.GetLongitude()), "text/plain");
    }
  }

  static void GetPhotoTag(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Photo photo;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid))
    {
      call.GetOutput().AnswerBuffer(photo.GetTag(), "text/plain");
    }
  }

  static void GetPhotoSecondsSinceEpoch(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Photo photo;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid))
    {
      call.GetOutput().AnswerBuffer(boost::lexical_cast<std::string>(photo.GetSecondsSinceEpoch()), "text/plain");
    }
  }

  static void PostSite(Orthanc::RestApiPostCall& call)
  {
    Json::Value request;
    if (call.ParseJsonRequest(request))
    {
      Site site = Site::FromJson(request);
      site.SetUuid(Orthanc::Toolbox::GenerateUuid());
      PhotoTrackApi::GetDatabaseWrapper(call).CreateOrUpdateSite(site);

      Json::Value answer = Json::objectValue;
      answer["SiteId"] = site.GetUuid();
      call.GetOutput().AnswerJson(answer);
    }
  }

  static void PostPhoto(Orthanc::RestApiPostCall& call)
  {
    Json::Value request;
    if (call.ParseJsonRequest(request))
    {
      Photo photo = Photo::FromJson(request);
      photo.SetUuid(Orthanc::Toolbox::GenerateUuid());
      PhotoTrackApi::GetDatabaseWrapper(call).CreateOrUpdatePhoto(photo);

      if (request.isMember("ImageData") && request.isMember("ImageMime"))
      {
        std::string result;
        Orthanc::Toolbox::DecodeBase64(result, request["ImageData"].asString());
        PhotoTrackApi::GetDatabaseWrapper(call).ReplaceImage
          (photo.GetUuid(), result, request["ImageMime"].asString());
      }

      Json::Value answer = Json::objectValue;
      answer["PhotoId"] = photo.GetUuid();
      call.GetOutput().AnswerJson(answer);
    }
  }

  static void PostUser(Orthanc::RestApiPostCall& call)
  {
    Json::Value request;
    if (call.ParseJsonRequest(request))
    {
      User user = User::FromJson(request);
      user.SetUuid(Orthanc::Toolbox::GenerateUuid());
      PhotoTrackApi::GetDatabaseWrapper(call).CreateOrUpdateUser(user);

      Json::Value answer = Json::objectValue;
      answer["UserId"] = user.GetUuid();
      call.GetOutput().AnswerJson(answer);
    }
  }

  static void UpdateSite(Orthanc::RestApiPutCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Json::Value request;
    if (call.ParseJsonRequest(request))
    {
      Site site;
      PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid);
      site.UpdateWithJson(request);
      PhotoTrackApi::GetDatabaseWrapper(call).CreateOrUpdateSite(site);
      call.GetOutput().AnswerBuffer("{}", "application/json");
    }
  }

  static void UpdatePhoto(Orthanc::RestApiPutCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Json::Value request;
    if (call.ParseJsonRequest(request))
    {
      Photo photo;
      PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid);
      photo.UpdateWithJson(request);
      PhotoTrackApi::GetDatabaseWrapper(call).CreateOrUpdatePhoto(photo);
      call.GetOutput().AnswerBuffer("{}", "application/json");
    }
  }

  static void UpdateUser(Orthanc::RestApiPutCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Json::Value request;
    if (call.ParseJsonRequest(request))
    {
      User user;
      PhotoTrackApi::GetDatabaseWrapper(call).GetUser(user, uuid);
      user.UpdateWithJson(request);
      PhotoTrackApi::GetDatabaseWrapper(call).CreateOrUpdateUser(user);
      call.GetOutput().AnswerBuffer("{}", "application/json");
    }
  }

  static void DeleteSite(Orthanc::RestApiDeleteCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");
    PhotoTrackApi::GetDatabaseWrapper(call).DeleteSite(uuid);
    call.GetOutput().AnswerBuffer("{}", "application/json");
  }

  static void GetImage(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    Photo photo;
    if (PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, uuid))
    {
      std::string image;
      PhotoTrackApi::GetFileStorage(call).ReadFile(image, photo.GetImageUuid());
      call.GetOutput().AnswerBuffer(image, photo.GetImageMime());
    }
  }
    
  static void DeletePhoto(Orthanc::RestApiDeleteCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");
    PhotoTrackApi::GetDatabaseWrapper(call).DeletePhoto(uuid);
    call.GetOutput().AnswerBuffer("{}", "application/json");
  }


  static void SetImage(Orthanc::RestApiPutCall& call)
  {
    std::string contentType = call.GetHttpHeader("content-type", "image/jpeg");

    std::string uuid = call.GetUriComponent("uuid", "");
    PhotoTrackApi::GetDatabaseWrapper(call).ReplaceImage
      (uuid, call.GetPutBody(), contentType);

    call.GetOutput().AnswerBuffer("{}", "application/json");
  }
  
  static void DeleteUser(Orthanc::RestApiDeleteCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");
    PhotoTrackApi::GetDatabaseWrapper(call).DeleteUser(uuid);
    call.GetOutput().AnswerBuffer("{}", "application/json");
  }


  static void GetSiteArchive(Orthanc::RestApiGetCall& call)
  {
    std::string uuid = call.GetUriComponent("uuid", "");

    // Create a RAII for the temporary file to manage the ZIP file
    Orthanc::Toolbox::TemporaryFile tmp;

    Site site;
    {    
      // Create a ZIP writer
      Orthanc::HierarchicalZipWriter writer(tmp.GetPath().c_str());
      writer.SetZip64(false);

      if (PhotoTrackApi::GetDatabaseWrapper(call).GetSite(site, uuid))
      {
        Json::Value lst;
        PhotoTrackApi::GetDatabaseWrapper(call).GetPhotos(lst, site);

        for (Json::Value::ArrayIndex i = 0; i < lst.size(); i++)
        {
          Photo photo;
          PhotoTrackApi::GetDatabaseWrapper(call).GetPhoto(photo, lst[i]["Uuid"].asString());
        
          std::string image;
          PhotoTrackApi::GetFileStorage(call).ReadFile(image, photo.GetImageUuid());

          std::string extension = ".jpg";
          if (photo.GetImageMime() == "image/png")
          {
            extension = ".png";
          }
          else if (photo.GetImageMime() == "plain/text")
          {
            extension = ".txt";
          }

          std::string filename = photo.GetTime() + " " + photo.GetTag() + extension;
          std::cout << filename << std::endl;

          writer.OpenFile(filename.c_str());
          writer.Write(image);
        }
      }
    }

    // Prepare the sending of the ZIP file
    Orthanc::FilesystemHttpSender sender(tmp.GetPath().c_str());
    sender.SetContentType("application/zip");

    std::string s = site.GetPitNumber();
    if (s.empty())
    {
      s = "SiteArchive";
    }

    sender.SetDownloadFilename(s + ".zip");

    // Send the ZIP
    call.GetOutput().AnswerFile(sender);
  }


  static void GetSinceAndLimit(int64_t& since,
                               unsigned int& limit,
                               bool& last,
                               const Orthanc::RestApiGetCall& call)
  {
    static const unsigned int MAX_RESULTS = 10;
    
    if (call.HasArgument("last"))
    {
      last = true;
      return;
    }

    last = false;

    try
    {
      since = boost::lexical_cast<int64_t>(call.GetArgument("since", "0"));
      limit = boost::lexical_cast<unsigned int>(call.GetArgument("limit", "0"));
    }
    catch (boost::bad_lexical_cast)
    {
      return;
    }

    if (limit == 0 || limit > MAX_RESULTS)
    {
      limit = MAX_RESULTS;
    }
  }


  static void ListChanges(Orthanc::RestApiGetCall& call)
  {
    DatabaseWrapper& db = PhotoTrackApi::GetDatabaseWrapper(call);

    int64_t since;
    unsigned int limit;
    bool last;
    GetSinceAndLimit(since, limit, last, call);

    Json::Value result;
    if (last)
    {
      db.GetLastChange(result);
    }
    else
    {
      db.GetChanges(result, since, limit);
    }

    call.GetOutput().AnswerJson(result);
  }


  PhotoTrackApi::PhotoTrackApi(bool isTest) : db_(NULL)
  {
    if (isTest)
    {
      Register("/sessions", GetSessions);
    }

    Register("/", RedirectToAssets);
    Register("/sessions", PostSession);
    Register("/sessions/{id}", DeleteSession);
    Register("/sessions/{id}", GetSessionProperties);
    Register("/sessions/{id}/{property}", GetSessionPropertyValue);

    Register("/sites", ListSites);
    Register("/sites", PostSite);
    Register("/sites/{uuid}", GetSite);
    Register("/sites/{uuid}", UpdateSite);
    Register("/sites/{uuid}", DeleteSite);
    Register("/sites/{uuid}/latitude", GetSiteLatitude);
    Register("/sites/{uuid}/longitude", GetSiteLongitude);
    Register("/sites/{uuid}/pit-number", GetSitePitNumber);
    Register("/sites/{uuid}/seconds-since-epoch", GetSiteSecondsSinceEpoch);
    Register("/sites/{uuid}/address", GetSiteAddress);
    Register("/sites/{uuid}/name", GetSiteName);
    Register("/sites/{uuid}/status", GetSiteStatus);    
    Register("/sites/{uuid}/archive", GetSiteArchive);    
    
    Register("/sites/{uuid}/photos", ListPhotosOfSite);

    Register("/photos", ListPhotos);
    Register("/photos", PostPhoto);
    Register("/photos/{uuid}", GetPhoto);
    Register("/photos/{uuid}", UpdatePhoto);
    Register("/photos/{uuid}", DeletePhoto);
    
    Register("/photos/{uuid}/latitude", GetPhotoLatitude);
    Register("/photos/{uuid}/longitude", GetPhotoLongitude);
    Register("/photos/{uuid}/tag", GetPhotoTag);
    Register("/photos/{uuid}/seconds-since-epoch", GetPhotoSecondsSinceEpoch);

    Register("/users", ListUsers);

    Register("/users", PostUser);
    Register("/users/{uuid}", GetUser);
    Register("/users/{uuid}", UpdateUser);
    Register("/users/{uuid}", DeleteUser);

    Register("/users/{uuid}/username", GetUserUserName);

    if (isTest)
    {
      Register("/users/{uuid}/password", GetUserPassword);
    }

    Register("/users/{uuid}/full-name", GetUserFullName);
    Register("/users/{uuid}/email", GetUserEmail);
    Register("/users/{uuid}/is-supervisor", GetUserIsSupervisor);
    Register("/users/{uuid}/is-admin", GetUserIsAdmin);
    Register("/users/{uuid}/organization", GetUserOrganization);

    Register("/photos/{uuid}/image", GetImage);
    Register("/photos/{uuid}/image", SetImage);

    Register("/changes", ListChanges);
  }
}
