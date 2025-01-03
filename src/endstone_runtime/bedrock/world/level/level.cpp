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

#include "bedrock/world/level/level.h"

#include "bedrock/world/level/gameplay_user_manager.h"
#include "endstone/detail/hook.h"
#include "endstone/detail/scheduler/scheduler.h"
#include "endstone/detail/server.h"

using endstone::detail::EndstoneScheduler;
using endstone::detail::EndstoneServer;

void Level::tick()
{
    static std::string function_decorated_name = __FUNCDNAME__;
    auto &server = entt::locator<EndstoneServer>::value();
    server.tick(getCurrentServerTick().tick_id,
                [&]() { ENDSTONE_HOOK_CALL_ORIGINAL_NAME(&Level::tick, function_decorated_name, this); });
}

bool Level::isUsableLevel(const ILevel &level)
{
    return !level.getTearingDown();
}
