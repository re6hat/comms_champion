//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include <string>

#include <QtCore/QtPlugin>

namespace comms_champion
{

class Plugin
{
public:
    virtual ~Plugin() = default;

    void initialize()
    {
        initializeImpl();
    }

    void finalize()
    {
        finalizeImpl();
    }

    void configure(const std::string& config = std::string())
    {
        configureImpl(config);
    }


protected:
    virtual void initializeImpl() = 0;
    virtual void finalizeImpl() = 0;
    virtual void configureImpl(const std::string& config) = 0;
};

}  // namespace comms_champion

Q_DECLARE_INTERFACE(comms_champion::Plugin, "cc.Plugin")


