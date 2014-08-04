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

#include "IClonable.h"

#include <string>
#include <map>
#include <list>

namespace PhotoTrack
{
  class PropertyMap : public IClonable
  {
  private:
    typedef std::map<std::string, std::string>  Properties;

    Properties properties_;

  public:
    IClonable* Clone() const;

    void SetValue(const std::string& property,
                  const std::string& value)
    {
      properties_[property] = value;
    }

    std::string GetValue(const std::string& property) const;

    void ListProperties(std::list<std::string>& result) const;
  };
}
