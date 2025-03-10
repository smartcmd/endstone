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

#include "bedrock/scripting/server_script_manager.h"

#include <mutex>
#include <thread>

#include "endstone/core/server.h"
#include "endstone/runtime/hook.h"

void ServerScriptManager::_runPlugins(PluginExecutionGroup exe_group, ServerInstance &server_instance)
{
    ENDSTONE_HOOK_CALL_ORIGINAL(&ServerScriptManager::_runPlugins, this, exe_group, server_instance);
    static std::once_flag init_server, init_level;
    switch (exe_group) {
    case PluginExecutionGroup::PrePackLoadExecution: {
        std::call_once(init_server, [&server_instance]() {
            auto &server = entt::locator<endstone::core::EndstoneServer>::value_or();
            server.init(server_instance);
        });
        break;
    }
    case PluginExecutionGroup::ServerStartExecution: {
        std::call_once(init_level, [&server_instance]() {
            auto &server = entt::locator<endstone::core::EndstoneServer>::value();
            server.setLevel(*server_instance.getMinecraft()->getLevel());
        });
        break;
    }
    default:
        break;
    }
}
