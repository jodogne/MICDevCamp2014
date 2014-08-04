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
#include "ActiveSessions.h"

#include "Toolbox.h"

#include <Core/OrthancException.h>
#include <Core/Uuid.h>
#include <glog/logging.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>   // For std::auto_ptr

namespace PhotoTrack
{
  ActiveSessions::ActiveSession::ActiveSession(IAuthenticator& authenticator,
                                               const std::string& username)
  {
    uuid_ = Orthanc::Toolbox::GenerateUuid();
    time_ = Toolbox::Now();
    username_ = username;
    payload_ = authenticator.GetUserPayload(username);
  }


  ActiveSessions::ActiveSession::~ActiveSession()
  {
    if (payload_ != NULL)
    {
      delete payload_;
    }
  }

  IClonable* ActiveSessions::ActiveSession::GetPayload() const
  {
    using namespace Orthanc;

    if (payload_ != NULL)
    {
      return payload_->Clone();
    }
    else
    {
      return NULL;
    }
  }


  uint64_t  ActiveSessions::ActiveSession::GetAge() const
  {
    //std::cout << Now() << " " << time_ << " " << (Now() - time_).total_milliseconds() << std::endl;
    return (Toolbox::Now() - time_).total_milliseconds();
  }

  void ActiveSessions::CloseAllSessions()
  {
    // The mutex must have been locked at this point

    for (SessionIndex::iterator it = index_.begin(); 
         it != index_.end(); ++it)
    {
      delete it->second;
    }

    history_.clear();
  }

  void ActiveSessions::CloseSession(const std::string& sessionId)
  {
    boost::mutex::scoped_lock lock(mutex_);

    LOG(WARNING) << "Closing session " << sessionId;

    SessionIndex::iterator session = index_.find(sessionId);

    if (session == index_.end())
    {
      // This session is already closed
      return;
    }

    delete session->second;
    index_.erase(session);
  }


  void ActiveSessions::CloseExpiredSessions()
  {
    // The mutex must have been locked at this point

    for (;;)
    {
      if (history_.empty())
      {
        // No more active sessions, giving up
        return;
      }

      // Retrieve the oldest active session
      std::string oldest = history_.front();

      SessionIndex::iterator session = index_.find(oldest);
      if (session == index_.end())
      {
        // This session was previously manually closed
      }
      else
      {
        if (session->second->GetAge() <= maxAge_)
        {
          // The oldest session is not old enough yet to expire, we
          // are done
          return;
        }

        delete session->second;
        index_.erase(session);
      }

      // Remove the oldest session
      history_.pop_front();
    }
  }

  
  bool ActiveSessions::IsActive(const std::string& session)
  {
    boost::mutex::scoped_lock lock(mutex_);

    CloseExpiredSessions();

    return index_.find(session) != index_.end();
  }


  IClonable* ActiveSessions::GetPayload(const std::string& session)
  {
    boost::mutex::scoped_lock lock(mutex_);

    CloseExpiredSessions();

    SessionIndex::const_iterator it = index_.find(session);
    if (it != index_.end())
    {
      return it->second->GetPayload();
    }
    else 
    {
      return NULL;
    }
  }


  bool ActiveSessions::OpenSession(std::string& sessionId,
                                   const std::string& username,
                                   const std::string& password)
  {
    using namespace Orthanc;

    if (authenticator_ == NULL)
    {
      LOG(ERROR) << "No authentification method has been set";
      throw OrthancException(ErrorCode_BadSequenceOfCalls);
    }

    if (!authenticator_->Authenticate(username, password))
    {
      LOG(INFO) << "Bad user credentials";
      return false;
    }

    std::auto_ptr<ActiveSession> session(new ActiveSession(*authenticator_, username));

    sessionId = session->GetId();

    {
      boost::mutex::scoped_lock lock(mutex_);
      history_.push_back(sessionId);
      index_[sessionId] = session.release();
    }

    return true;
  }


  void ActiveSessions::ListSessions(std::list<std::string>& result)
  {
    result.clear();
    
    boost::mutex::scoped_lock lock(mutex_);
    
    CloseExpiredSessions();

    for (SessionIndex::const_iterator it = index_.begin();
         it != index_.end(); it++)
    {
      result.push_back(it->second->GetId());
    }
  }


  void ActiveSessions::SetMaxAge(uint64_t age)
  {
    boost::mutex::scoped_lock lock(mutex_);
    maxAge_ = age;
    CloseExpiredSessions();
  }


  IAuthenticator& ActiveSessions::GetAuthenticator() 
  {
    boost::mutex::scoped_lock lock(mutex_);

    if (authenticator_ == NULL)
    {
      LOG(ERROR) << "No authentification method has been set";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    return *authenticator_;
  }

  void ActiveSessions::SetAuthenticator(IAuthenticator& authenticator)
  {
    boost::mutex::scoped_lock lock(mutex_);

    authenticator_ = &authenticator;

    CloseAllSessions();
  }

  static std::string FormatExpirationTime(uint64_t age)
  {
    // Use "Expires" instead of "Max-Age" to work with all browsers
    // http://mrcoles.com/blog/cookies-max-age-vs-expires/

    // Get the expiration time in GMT/UTC
    boost::posix_time::ptime expiration = Toolbox::Now() + 
      boost::posix_time::milliseconds(age) - Toolbox::GetUtcOffset();

    // Format the date according to RFC 1123
    // http://en.wikipedia.org/wiki/HTTP_cookie#Expires_and_Max-Age
    return Toolbox::FormatTime(expiration, "%a, %d-%b-%y %H:%M:%S GMT");
  }

  std::string ActiveSessions::FormatCookie(const std::string& sessionId) const
  {
    // Set-Cookie: lu=Rg3vHJZnehYLjVg7qi3bZjzg; Expires=Tue, 15-Jan-2013 21:47:38 GMT; Path=/; Domain=.example.com; HttpOnly
    return sessionId + "; Path=/; Expires=" + FormatExpirationTime(maxAge_);
  }

  std::string ActiveSessions::FormatResetCookie() const
  {
    // This marks the cookie to be deleted by the browser in 1 second,
    // and before it actually gets deleted, its value is set to the
    // empty string
    return "; Path=/; Expires=" + FormatExpirationTime(1000);
  }

}
