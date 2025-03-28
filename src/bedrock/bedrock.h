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


#if _WIN32
#define ENDSTONE_HOOK __declspec(dllexport)
#elif __linux__
#define ENDSTONE_HOOK __attribute__((visibility("default")))
#endif
#define ENDSTONE_VHOOK

#ifdef NDEBUG
#ifdef _WIN32
#define BEDROCK_STATIC_ASSERT_SIZE(className, sizeWindows, sizeLinux) \
    static_assert(sizeof(className) == sizeWindows, "Size of " #className " does not match expected size.")
#else
#define BEDROCK_STATIC_ASSERT_SIZE(className, sizeWindows, sizeLinux) \
    static_assert(sizeof(className) == sizeLinux, "Size of " #className " does not match expected size.")
#endif
#else
#define BEDROCK_STATIC_ASSERT_SIZE(className, sizeWindows, sizeLinux)
#endif
