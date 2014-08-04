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

#include "IAuthenticator.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/thread.hpp>
#include <list>

namespace PhotoTrack
{
  class ActiveSessions : public boost::noncopyable
  {
  private:
    class ActiveSession
    {
    private:
      std::string               uuid_;
      boost::posix_time::ptime  time_;
      std::string               username_;
      IClonable                *payload_;

    public:
      ActiveSession(IAuthenticator& authenticator,
                    const std::string& username);

      ~ActiveSession();

      const std::string& GetId() const
      {
        return uuid_;
      }

      IClonable* GetPayload() const;

      uint64_t  GetAge() const;
    };

    typedef std::map<std::string, ActiveSession*>  SessionIndex;
    
    boost::mutex            mutex_;
    uint64_t                maxAge_;   // In milliseconds
    IAuthenticator         *authenticator_;
    std::list<std::string>  history_;  // Ordered by age
    SessionIndex            index_;

    void CloseAllSessions();

    void CloseExpiredSessions();

  public:
    ActiveSessions() : 
      maxAge_(60 * 60 * 1000),    // 1 hour
      authenticator_(NULL)
    {
    }

    ~ActiveSessions()
    {
      CloseAllSessions();
    }

    uint64_t GetMaxAge() const
    {
      return maxAge_;
    }

    void SetMaxAge(uint64_t age);

    IAuthenticator& GetAuthenticator();

    void SetAuthenticator(IAuthenticator& authenticator);

    void CloseSession(const std::string& session);

    bool IsActive(const std::string& session);

    IClonable* GetPayload(const std::string& session);

    bool OpenSession(std::string& session,
                     const std::string& username,
                     const std::string& password);

    void ListSessions(std::list<std::string>& result);

    std::string FormatCookie(const std::string& sessionId) const;

    std::string FormatResetCookie() const;
  };
}
