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

#pragma once

#include "endstone/event/actor/actor_event.h"

namespace endstone {

/**
 * @brief Called when an Actor is removed.
 *
 * This event should only be used for monitoring. Modifying the actor during or after this event leads to undefined
 * behaviours. This event will not be called for Players.
 */
class ActorRemoveEvent : public ActorEvent<Actor> {
public:
    using ActorEvent::ActorEvent;

    inline static const std::string NAME = "ActorRemoveEvent";
    [[nodiscard]] std::string getEventName() const override
    {
        return NAME;
    }

    // TODO(event): add remove cause
};

}  // namespace endstone
