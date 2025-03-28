// Copyright (c) 2023, The Endstone Project. (https://endstone.dev) All Rights Reserved.
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

#include <cstdint>

#include <gtest/gtest.h>

#include "bedrock/platform/uuid.h"
#include "endstone/core/util/uuid.h"

TEST(UUIDTest, ConvertToAndFromEndstone)
{
    endstone::UUID expect;
    for (std::uint8_t i = 0; i < 16; ++i) {
        expect.data[i] = i;  // Initialization for test
    }

    const auto mce_uuid = endstone::core::EndstoneUUID::toMinecraft(expect);
    const auto actual = endstone::core::EndstoneUUID::fromMinecraft(mce_uuid);

    for (std::uint8_t i = 0; i < 16; ++i) {
        EXPECT_EQ(expect.data[i], actual.data[i]);
    }
}
