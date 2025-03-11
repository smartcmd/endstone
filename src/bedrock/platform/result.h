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

#include <system_error>

#include <nonstd/expected.hpp>

#include "bedrock/bedrock.h"
#include "bedrock/platform/check.h"

namespace Bedrock {

template <typename T, typename E = std::error_code>
class Result : nonstd::expected<T, ErrorInfo<E>> {
public:
    template <typename ErrorTypeU>
    Result(ErrorInfo<ErrorTypeU> error_info) : nonstd::expected<T, ErrorInfo<ErrorTypeU>>(error_info)
    {
    }
};
BEDROCK_STATIC_ASSERT_SIZE(Result<unsigned char>, 72, 72);

}  // namespace Bedrock
