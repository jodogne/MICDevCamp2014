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

#include "PropertyMap.h"

#include <Core/OrthancException.h>

#include <memory>

namespace PhotoTrack
{
  IClonable* PropertyMap::Clone() const
  {
    std::auto_ptr<PropertyMap> cloned(new PropertyMap);
    cloned->properties_ = properties_;
    return cloned.release();
  }

  std::string PropertyMap::GetValue(const std::string& property) const
  {
    Properties::const_iterator it = properties_.find(property);

    if (it == properties_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      return it->second;
    }
  }


  void PropertyMap::ListProperties(std::list<std::string>& result) const
  {
    result.clear();

    for (Properties::const_iterator it = properties_.begin();
         it != properties_.end(); it++)
    {
      result.push_back(it->first);
    }
  }
}
