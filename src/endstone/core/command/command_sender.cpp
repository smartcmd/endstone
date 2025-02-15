// Copyright (c) 2024, The Endstone Project. (https://endstone.dev) All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "bedrock/server/commands/command_origin.h"

#include <entt/entt.hpp>

#include "bedrock/server/commands/command_origin_loader.h"
#include "bedrock/world/actor/actor.h"
#include "bedrock/world/actor/player/player.h"
#include "endstone/command/command_sender.h"
#include "endstone/core/command/console_command_sender.h"
#include "endstone/core/level/level.h"
#include "endstone/core/player.h"
#include "endstone/core/server.h"

using endstone::core::EndstoneActor;
using endstone::core::EndstoneConsoleCommandSender;
using endstone::core::EndstoneLevel;
using endstone::core::EndstonePlayer;
using endstone::core::EndstoneServer;

std::shared_ptr<endstone::CommandSender> CommandOrigin::getEndstoneSender() const
{
    auto &server = entt::locator<EndstoneServer>::value();
    switch (getOriginType()) {
    case CommandOriginType::DedicatedServer: {
        return std::static_pointer_cast<endstone::ConsoleCommandSender>(
            static_cast<EndstoneConsoleCommandSender &>(server.getCommandSender()).shared_from_this());
    }
    case CommandOriginType::Player:
    case CommandOriginType::Entity: {
        return std::static_pointer_cast<endstone::Actor>(getEntity()->getEndstoneActor().shared_from_this());
    }
    case CommandOriginType::Virtual:
        // TODO(command): we need ProxiedCommandSender, getOrigin will return the callee, getOutputReceiver will return
        //   the caller
    case CommandOriginType::CommandBlock:
    case CommandOriginType::MinecartCommandBlock:
        // TODO(permission): add BlockCommandSender, Entity and CommandMinecart
    default:
        return nullptr;
    }
}