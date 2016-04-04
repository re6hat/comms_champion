//
// Copyright 2015 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "comms_champion/PluginControlInterface.h"

#include <cassert>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QObject>
CC_ENABLE_WARNINGS()

namespace comms_champion
{

PluginControlInterface::PluginControlInterface() = default;
PluginControlInterface::PluginControlInterface(const PluginControlInterface&) = default;

PluginControlInterface::~PluginControlInterface() = default;

}  // namespace comms_champion

