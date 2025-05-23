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

#include <string>

#include "bedrock/core/utility/observer.h"

namespace Social {
class XboxLiveUserObserver : public Core::Observer<XboxLiveUserObserver, Core::SingleThreadedLock> {
public:
    virtual void onXboxUserBlocked(std::string const &) = 0;
    virtual void onXboxUserUnblocked(std::string const &) = 0;
};
static_assert(sizeof(XboxLiveUserObserver) == 16);
}  // namespace Social
