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

#include "ServerTestsPrecompiledHeaders.h"

#include "../ApplicationSources/Database.h"

#include <Core/Toolbox.h>
#include <Core/Uuid.h>
#include <Core/SQLite/Connection.h>

#include <gtest/gtest.h>
#include <glog/logging.h>

using namespace Orthanc;


TEST(Uuid, Generation)
{
  for (int i = 0; i < 10; i++)
  {
    std::string s = Toolbox::GenerateUuid();
    ASSERT_TRUE(Toolbox::IsUuid(s));
  }
}

TEST(Uuid, Test)
{
  ASSERT_FALSE(Toolbox::IsUuid(""));
  ASSERT_FALSE(Toolbox::IsUuid("012345678901234567890123456789012345"));
  ASSERT_TRUE(Toolbox::IsUuid("550e8400-e29b-41d4-a716-446655440000"));
  ASSERT_FALSE(Toolbox::IsUuid("550e8400-e29b-41d4-a716-44665544000_"));
  ASSERT_FALSE(Toolbox::IsUuid("01234567890123456789012345678901234_"));
  ASSERT_FALSE(Toolbox::StartsWithUuid("550e8400-e29b-41d4-a716-44665544000"));
  ASSERT_TRUE(Toolbox::StartsWithUuid("550e8400-e29b-41d4-a716-446655440000"));
  ASSERT_TRUE(Toolbox::StartsWithUuid("550e8400-e29b-41d4-a716-446655440000 ok"));
  ASSERT_FALSE(Toolbox::StartsWithUuid("550e8400-e29b-41d4-a716-446655440000ok"));
}

TEST(Toolbox, IsSHA1)
{
  ASSERT_FALSE(Toolbox::IsSHA1(""));
  ASSERT_FALSE(Toolbox::IsSHA1("01234567890123456789012345678901234567890123"));
  ASSERT_FALSE(Toolbox::IsSHA1("012345678901234567890123456789012345678901234"));
  ASSERT_TRUE(Toolbox::IsSHA1("b5ed549f-956400ce-69a8c063-bf5b78be-2732a4b9"));

  std::string s;
  Toolbox::ComputeSHA1(s, "The quick brown fox jumps over the lazy dog");
  ASSERT_TRUE(Toolbox::IsSHA1(s));
  ASSERT_EQ("2fd4e1c6-7a2d28fc-ed849ee1-bb76e739-1b93eb12", s);

  ASSERT_FALSE(Toolbox::IsSHA1("b5ed549f-956400ce-69a8c063-bf5b78be-2732a4b_"));
}

TEST(SQLite, Connection)
{
  Toolbox::RemoveFile("UnitTestsResults/coucou");
  SQLite::Connection c;
  c.Open("UnitTestsResults/coucou");
  c.Execute("CREATE TABLE c(k INTEGER PRIMARY KEY AUTOINCREMENT, v INTEGER)");
  c.Execute("INSERT INTO c VALUES(NULL, 42);");
}


TEST(Database, Site)
{
  Toolbox::RemoveFile("test.db");
  Orthanc::FileStorage storage("UnitTestsStorage");
  DatabaseWrapper db("test.db", storage);

  Site site;
  db.CreateOrUpdateSite(site);

  site.SetGps(42, 44);
  db.CreateOrUpdateSite(site);

  Photo photo;
  photo.SetSiteUuid(site.GetUuid());
  db.CreateOrUpdatePhoto(photo);
}


TEST(Database, Mockup)
{
  Toolbox::RemoveFile("mockup.db");
  Orthanc::FileStorage storage("MockupStorage");
  DatabaseWrapper db("mockup.db", storage);

  std::list<std::string>  sites;
  unsigned int count = 0;
  for (unsigned int i = 0; i < 6; i++)
  {
    Site site;
    site.SetPitNumber("pit " + boost::lexical_cast<std::string>(i));
    site.SetName("mockup site " + boost::lexical_cast<std::string>(i));
    site.SetAddress("Liege");
    site.SetGps(static_cast<float>(i), static_cast<float>(10 * i));
    db.CreateOrUpdateSite(site);
    sites.push_back(site.GetUuid());

    for (unsigned int j = 0; j < 10; j++, count++)
    {
      Photo photo;
      photo.SetSite(site);
      photo.SetTag("My site is pit " + boost::lexical_cast<std::string>(i));
      photo.SetGps(static_cast<float>(i + j), static_cast<float>(10 * i + j));
      db.CreateOrUpdatePhoto(photo);

      std::string s = boost::lexical_cast<std::string>(count) + "\n";
      db.ReplaceImage(photo.GetUuid(), s, "plain/text");
    }
  }

  /*for (std::list<std::string>::const_iterator
         it = sites.begin(); it != sites.end(); it++)
  {
    db.DeleteSite(*it);
    }*/

  db.DeleteSite(sites.front());
}


TEST(Database, SiteSerialization)
{
  Site site;
  std::string uuid = site.GetUuid();

  site.SetPitNumber("pit 42");
  site.SetName("mockup site");
  site.SetAddress("Liege");
  site.SetGps(10, 20);

  Json::Value v;
  site.ToJson(v);

  Site site2 = Site::FromJson(v);

  ASSERT_EQ(uuid, site2.GetUuid());
  ASSERT_EQ("pit 42", site2.GetPitNumber());
  ASSERT_EQ("mockup site", site2.GetName());
  ASSERT_EQ("Liege", site2.GetAddress());
  ASSERT_TRUE(site2.HasGps());
  ASSERT_EQ(10, site2.GetLatitude());
  ASSERT_EQ(20, site2.GetLongitude());
}

TEST(Database, UserSerialization)
{
  User user;
  std::string uuid = user.GetUuid();

  user.SetFullName("Tôôt Pûût");
  user.SetEmail("john@mic-prout.be");
  user.SetPassword("@|#{@é#é{é[é€@€€#{");
  user.SetIsSupervisor(true);
  user.SetIsAdmin(false);
  user.SetOrganization("ACME");
  
  Json::Value v;
  user.ToJson(v);

  User user2 = User::FromJson(v);

  ASSERT_EQ(uuid, user2.GetUuid());
  ASSERT_EQ("Tôôt Pûût", user2.GetFullName());
  ASSERT_EQ("john@mic-prout.be", user2.GetEmail());
  ASSERT_EQ("@|#{@é#é{é[é€@€€#{", user2.GetPassword());
  ASSERT_TRUE(user2.IsSupervisor());
  ASSERT_FALSE(user2.IsAdmin());
  ASSERT_EQ("ACME", user2.GetOrganization());
}

TEST(Database, PhotoSerialization)
{
Photo photo;
std::string uuid = photo.GetUuid();

photo.SetGps(10.12345f, 20.98765f);
photo.SetSecondsSinceEpoch(456781);
photo.SetTag("préèt");
photo.SetImageMime("image/png");

Json::Value v;
photo.ToJson(v);

Photo photo2 = Photo::FromJson(v);

ASSERT_EQ(uuid, photo2.GetUuid());
ASSERT_EQ(10.12345f, photo2.GetLatitude());
ASSERT_EQ(20.98765f, photo2.GetLongitude());
ASSERT_EQ(456781, photo2.GetSecondsSinceEpoch());
ASSERT_EQ("préèt", photo2.GetTag());
ASSERT_EQ("image/png", photo.GetImageMime());
}

TEST(Database, DeleteSites)
{
  Toolbox::RemoveFile("test.db");
  Orthanc::FileStorage storage("UnitTestsStorage");
  DatabaseWrapper db("test.db", storage);

  for (unsigned int i = 0; i < 3; i++)
  {
    Site site;
    db.CreateOrUpdateSite(site);

    for (unsigned int j = 0; j < 4; j++)
    {
      Photo photo;
      photo.SetSite(site);
      db.CreateOrUpdatePhoto(photo);
    }
  }

  Json::Value l;
  db.GetPhotos(l);  ASSERT_EQ(3u * 4u, l.size());
  db.GetSites(l);   ASSERT_EQ(3u, l.size());

  db.DeleteSite(l[0]["Uuid"].asString());  // Delete 1 site

  db.GetPhotos(l); ASSERT_EQ(2u * 4u, l.size());
  db.GetSites(l);  ASSERT_EQ(2u, l.size());
  
  db.DeleteSite(l[1]["Uuid"].asString());  // Delete remaining sites
  db.DeleteSite(l[0]["Uuid"].asString());  // Delete remaining sites

  db.GetPhotos(l); ASSERT_EQ(0u, l.size());
  db.GetSites(l);  ASSERT_EQ(0u, l.size());
  
}



TEST(Cookie, Basic)
{
  // https://en.wikipedia.org/wiki/HTTP_cookie#Setting_a_cookie
  std::string s = " name=value; name2=value2  ;name3=value3  ";

  std::map<std::string, std::string> cookies;
  PhotoTrack::Toolbox::ParseCookies(cookies, s);

  ASSERT_EQ(3u, cookies.size());
  ASSERT_EQ("value", cookies["name"]);
  ASSERT_EQ("value2", cookies["name2"]);
  ASSERT_EQ("value3", cookies["name3"]);
}


int main(int argc, char **argv)
{
  // Initialize Google's logging library.
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;

  // Go to trace-level verbosity
  //FLAGS_v = 1;

  Toolbox::DetectEndianness();

  google::InitGoogleLogging("Orthanc");

  Toolbox::CreateDirectory("UnitTestsResults");

  ::testing::InitGoogleTest(&argc, argv);

  LOG(WARNING) << "Starting the tests";
  int result = RUN_ALL_TESTS();

  return result;
}
