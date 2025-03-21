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

#include "bedrock/locale/i18n.h"

#include "bedrock/symbol.h"

I18n &getI18n()
{
#ifdef _WIN32
    // TODO(fixme): avoid hardcoded value
    constexpr std::size_t offset = 0x60BBF20;  // 1.21.70-preview.26
    auto *address = reinterpret_cast<I18n *>(static_cast<char *>(endstone::detail::get_executable_base()) + offset);
    return *address;
#else
    return BEDROCK_CALL(&getI18n);
#endif
}
