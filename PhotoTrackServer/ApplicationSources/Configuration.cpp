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
#include "Configuration.h"

#include <Core/Toolbox.h>
#include <glog/logging.h>

#include <json/value.h>
#include <json/reader.h>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


namespace PhotoTrack
{
  namespace Configuration
  {
    static Json::Value globalConfiguration;
    static boost::mutex globalMutex;
    static boost::filesystem::path defaultDirectory;

    void Initialize(const char* filename)
    {
      if (filename == NULL)
      {
        globalConfiguration = Json::objectValue;
      }
      else
      {
        std::string content;
        defaultDirectory = boost::filesystem::path(filename).parent_path();
        Orthanc::Toolbox::ReadFile(content, filename);

        Json::Reader reader;
        if (!reader.parse(content, globalConfiguration))
        {
          LOG(ERROR) << "Unable to parse the configuration file";
          exit(-1);
        }
      }
    }


    void Finalize()
    {

    }


    bool HasParameter(const std::string& name)
    {
      boost::mutex::scoped_lock lock(globalMutex);
      return globalConfiguration.isMember(name);
    }


    int GetInteger(const std::string& name, 
                   int defaultValue)
    {
      boost::mutex::scoped_lock lock(globalMutex);

      if (globalConfiguration.isMember(name))
      {
        const Json::Value& v = globalConfiguration[name];

        if (v.type() == Json::intValue)
        {
          return v.asInt();
        }
    
        return boost::lexical_cast<int>(v.asString());
      }
      else
      {
        return defaultValue;
      }
    }


    std::string GetString(const std::string& name, 
                          const std::string& defaultValue)
    {
      boost::mutex::scoped_lock lock(globalMutex);

      if (globalConfiguration.isMember(name))
      {
        const Json::Value& v = globalConfiguration[name];
        return v.asString();
      }
      else
      {
        return defaultValue;
      }
    }


    static std::string InterpretRelativePath(const std::string& baseDirectory,
                                             const std::string& relativePath)
    {
      boost::filesystem::path base(baseDirectory);
      boost::filesystem::path relative(relativePath);

      /**
         The following lines should be equivalent to this one: 

         return (base / relative).string();

         However, for some unknown reason, some versions of Boost do not
         make the proper path resolution when "baseDirectory" is an
         absolute path. So, a hack is used below.
      **/

      if (relative.is_absolute())
      {
        return relative.string();
      }
      else
      {
        return (base / relative).string();
      }
    }


    std::string GetPath(const std::string& name, 
                        const std::string& defaultValue)
    {
      return InterpretRelativePath(defaultDirectory.string(), 
                                   GetString(name, defaultValue));
    }
  }
}
