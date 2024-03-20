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

#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

#include "endstone/command/command.h"
#include "endstone/detail/plugin/plugin_metadata.h"
#include "endstone/endstone.h"
#include "endstone/permissions/permission.h"
#include "endstone/plugin/plugin_load_order.h"

namespace endstone {

class PluginDescription {
public:
    PluginDescription(std::string name, std::string version, const detail::PluginMetadata &metadata = {})
    {
        name_ = std::move(name);
        std::replace(name_.begin(), name_.end(), ' ', '_');
        version_ = std::move(version);
        full_name_ = fmt::format("{} v{}", name_, version_);
        metadata_ = metadata;
    }

    /**
     * Gives the name of the plugin. This name is a unique identifier for plugins.
     *
     * @return the name of the plugin
     */
    [[nodiscard]] std::string getName() const
    {
        return name_;
    }

    /**
     * Gives the version of the plugin.
     *
     * @return the version of the plugin
     */
    [[nodiscard]] std::string getVersion() const
    {
        return version_;
    }

    /**
     * Returns the name of a plugin, including the version.
     *
     * @return a descriptive name of the plugin and respective version
     */
    [[nodiscard]] std::string getFullName() const
    {
        return full_name_;
    }

    /**
     * Gives the API version which this plugin is designed to support.
     *
     * @return the API version supported by the plugin
     */
    [[nodiscard]] std::string getAPIVersion() const
    {
        return ENDSTONE_TOSTRING(ENDSTONE_VERSION_MAJOR) "." ENDSTONE_TOSTRING(ENDSTONE_VERSION_MINOR);
    }

    /**
     * Gives a human-friendly description of the functionality the plugin provides.
     * @return description of this plugin, or empty if not specified
     */
    [[nodiscard]] std::string getDescription() const
    {
        return metadata_.description;
    }

    /**
     * Gives the phase of server startup that the plugin should be loaded.
     *
     * @return the phase when the plugin should be loaded
     */
    [[nodiscard]] PluginLoadOrder getLoad() const
    {
        return metadata_.load;
    }

    /**
     * Gives the list of authors for the plugin.
     *
     * @return an immutable list of the plugin's authors
     */
    [[nodiscard]] std::vector<std::string> getAuthors() const
    {
        return metadata_.authors;
    }

    /**
     * Gives the list of contributors for the plugin.
     *
     * @return an immutable list of the plugin's contributions
     */
    [[nodiscard]] std::vector<std::string> getContributors() const
    {
        return metadata_.contributors;
    }

    /**
     * Gives the plugin's or plugin's author's website.
     *
     * @return the website of this plugin, or empty if not specified
     */
    [[nodiscard]] std::string getWebsite() const
    {
        return metadata_.website;
    }

    /**
     * Gives the token to prefix plugin-specific logging messages with.
     *
     * @return the prefixed logging token, or empty if not specified
     */
    [[nodiscard]] std::string getPrefix() const
    {
        return metadata_.prefix;
    }

private:
    std::string name_;
    std::string version_;
    std::string full_name_;
    detail::PluginMetadata metadata_;
};

}  // namespace endstone
