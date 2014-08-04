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

#include "../ApplicationSources/ActiveSessions.h"

#include <Core/OrthancException.h>
#include <Core/Toolbox.h>
#include <gtest/gtest.h>

using namespace Orthanc;
using namespace PhotoTrack;

namespace
{
  class DynamicString : public IClonable
  {
  private:
    std::string value_;

  public:
    DynamicString(const std::string value) : value_(value)
    {
    }

    IClonable* Clone() const
    {
      return new DynamicString(value_);
    }

    const std::string& GetValue() const
    {
      return value_;
    }
  };

  class DummyAuthenticator : public IAuthenticator
  {
  private:
    bool  withPayload_;

  public:
    DummyAuthenticator(bool withPayload) : withPayload_(withPayload)
    {
    }

    virtual bool Authenticate(const std::string& username,
                              const std::string& password)
    {
      return (username == "user" &&
              password == "pass");
    }

    virtual IClonable* GetUserPayload(const std::string& username)
    {
      if (withPayload_)
      {
        return new DynamicString("Hello");
      }
      else
      {
        return NULL;
      }
    }
  };
}


TEST(ActiveSessions, Basic)
{
  ActiveSessions sessions;

  std::string s1, s2;
  ASSERT_THROW(sessions.OpenSession(s1, "user", "pass"), OrthancException);

  DummyAuthenticator auth(false);
  sessions.SetAuthenticator(auth);
  ASSERT_FALSE(sessions.OpenSession(s1, "user", "bad"));
  ASSERT_TRUE(sessions.OpenSession(s1, "user", "pass"));
  ASSERT_TRUE(sessions.OpenSession(s2, "user", "pass"));

  ASSERT_TRUE(sessions.IsActive(s1));
  ASSERT_TRUE(sessions.IsActive(s2));

  std::list<std::string> lst;
  sessions.ListSessions(lst);
  ASSERT_EQ(2u, lst.size());

  sessions.CloseSession(s1);

  ASSERT_FALSE(sessions.IsActive(s1));
  ASSERT_TRUE(sessions.IsActive(s2));

  sessions.ListSessions(lst);
  ASSERT_EQ(1u, lst.size());
  ASSERT_EQ(s2, lst.front());

  sessions.CloseSession(s2);

  ASSERT_FALSE(sessions.IsActive(s1));
  ASSERT_FALSE(sessions.IsActive(s2));
  sessions.ListSessions(lst);
  ASSERT_EQ(0u, lst.size());
}


TEST(ActiveSessions, Autoclose)
{
  ActiveSessions sessions;
  sessions.SetMaxAge(100);  // 100 milliseconds

  DummyAuthenticator auth(true);
  sessions.SetAuthenticator(auth);

  std::string s;
  sessions.OpenSession(s, "user", "pass");

  std::list<std::string> lst;
  for (;;)
  {
    sessions.ListSessions(lst);
    if (lst.size() == 0)
    {
      break;
    }

    Toolbox::USleep(10000);  // 10 milliseconds
  }

  ASSERT_FALSE(sessions.IsActive(s));
}


TEST(ActiveSessions, Payload)
{
  ActiveSessions sessions;

  DummyAuthenticator auth(true);
  sessions.SetAuthenticator(auth);

  std::string s;
  sessions.OpenSession(s, "user", "pass");

  std::auto_ptr<IClonable> p(sessions.GetPayload(s));
  ASSERT_TRUE(p.get() != NULL);
  ASSERT_EQ("Hello", dynamic_cast<DynamicString&>(*p).GetValue());

  sessions.CloseSession(s);
  ASSERT_FALSE(sessions.IsActive(s));

  ASSERT_TRUE(sessions.GetPayload(s) == NULL);
}


TEST(ActiveSessions, NoPayload)
{
  ActiveSessions sessions;

  DummyAuthenticator auth(false);
  sessions.SetAuthenticator(auth);

  std::string s;
  sessions.OpenSession(s, "user", "pass");

  ASSERT_TRUE(sessions.GetPayload(s) == NULL);

  sessions.CloseSession(s);
  ASSERT_FALSE(sessions.IsActive(s));

  ASSERT_TRUE(sessions.GetPayload(s) == NULL);
}
