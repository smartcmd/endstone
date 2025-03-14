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

#include "endstone/core/server.h"

#include <filesystem>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <pybind11/pybind11.h>

#include "bedrock/network/server_network_handler.h"
#include "bedrock/platform/threading/assigned_thread.h"
#include "bedrock/shared_constants.h"
#include "bedrock/world/level/block/block_descriptor.h"
#include "bedrock/world/scores/server_scoreboard.h"
#include "endstone/color_format.h"
#include "endstone/command/plugin_command.h"
#include "endstone/core/block/block_data.h"
#include "endstone/core/boss/boss_bar.h"
#include "endstone/core/command/command_map.h"
#include "endstone/core/command/console_command_sender.h"
#include "endstone/core/event/handlers/actor_gameplay_handler.h"
#include "endstone/core/event/handlers/block_gameplay_handler.h"
#include "endstone/core/event/handlers/level_gameplay_handler.h"
#include "endstone/core/event/handlers/player_gameplay_handler.h"
#include "endstone/core/event/handlers/scripting_event_handler.h"
#include "endstone/core/event/handlers/server_network_event_handler.h"
#include "endstone/core/level/level.h"
#include "endstone/core/logger_factory.h"
#include "endstone/core/message.h"
#include "endstone/core/permissions/default_permissions.h"
#include "endstone/core/plugin/cpp_plugin_loader.h"
#include "endstone/core/plugin/python_plugin_loader.h"
#include "endstone/core/signal_handler.h"
#include "endstone/core/util/error.h"
#include "endstone/core/util/uuid.h"
#include "endstone/event/server/broadcast_message_event.h"
#include "endstone/event/server/server_load_event.h"
#include "endstone/plugin/plugin.h"

namespace fs = std::filesystem;
namespace py = pybind11;

namespace endstone::core {

EndstoneServer::EndstoneServer() : logger_(LoggerFactory::getLogger("Server"))
{
    EndstoneServer::getLogger().info("{}This server is running {} version: {} (Minecraft: {})",
                                     ColorFormat::DarkAqua + ColorFormat::Bold, EndstoneServer::getName(),
                                     EndstoneServer::getVersion(), EndstoneServer::getMinecraftVersion());

    crash_handler_ = std::make_unique<CrashHandler>();
    signal_handler_ = std::make_unique<SignalHandler>();
    player_ban_list_ = std::make_unique<EndstonePlayerBanList>("banned-players.json");
    ip_ban_list_ = std::make_unique<EndstoneIpBanList>("banned-ips.json");
    language_ = std::make_unique<EndstoneLanguage>();
    plugin_manager_ = std::make_unique<EndstonePluginManager>(*this);
    service_manager_ = std::make_unique<EndstoneServiceManager>();
    command_sender_ = EndstoneConsoleCommandSender::create();
    scheduler_ = std::make_unique<EndstoneScheduler>(*this);
    start_time_ = std::chrono::system_clock::now();
}

EndstoneServer::~EndstoneServer()
{
    py::gil_scoped_acquire acquire{};
    disablePlugins();
    unregisterEventListeners();
}

void EndstoneServer::init(ServerInstance &server_instance)
{
    if (server_instance_) {
        throw std::runtime_error("Server instance already initialized.");
    }
    server_instance_ = &server_instance;
    command_sender_->init();
    player_ban_list_->load();
    ip_ban_list_->load();
    loadPlugins();
    enablePlugins(PluginLoadOrder::Startup);
}

void EndstoneServer::setLevel(::Level &level)
{
    if (level_) {
        throw std::runtime_error("Level already initialized.");
    }
    level_ = std::make_unique<EndstoneLevel>(level);
    scoreboard_ = std::make_unique<EndstoneScoreboard>(level.getScoreboard());
    command_map_ = std::make_unique<EndstoneCommandMap>(*this);
    loadResourcePacks();
    registerEventListeners();
    level._getPlayerDeathManager()->sender_.reset();  // prevent BDS from sending the death message
    enablePlugins(PluginLoadOrder::PostWorld);
    ServerLoadEvent event{ServerLoadEvent::LoadType::Startup};
    getPluginManager().callEvent(event);
}

void EndstoneServer::setResourcePackRepository(Bedrock::NotNullNonOwnerPtr<IResourcePackRepository> repo)
{
    if (resource_pack_repository_) {
        throw std::runtime_error("Resource pack repository already set.");
    }
    resource_pack_repository_ = std::move(repo);
    resource_pack_source_ = std::make_unique<EndstonePackSource>(
        resource_pack_repository_->getResourcePacksPath().getContainer(), PackType::Resources);
}

PackSource &EndstoneServer::getPackSource() const
{
    return *resource_pack_source_;
}

void EndstoneServer::loadResourcePacks()
{
    const auto *manager = level_->getHandle().getClientResourcePackManager();

    // Load zipped packs
    nlohmann::json json;
    resource_pack_source_->forEachPackConst([&json](auto &pack) {
        auto &identity = pack.getManifest().getIdentity();
        json.push_back({
            {"pack_id", identity.id.asString()},
            {"version", {identity.version.getMajor(), identity.version.getMinor(), identity.version.getPatch()}},
        });
    });
    std::stringstream ss(json.dump());
    const auto pack_stack = ResourcePackStack::deserialize(ss, *resource_pack_repository_);

    // Add encryption keys to network handler to be sent to clients
    auto content_keys = resource_pack_source_->getContentKeys();
    getServer().getMinecraft()->getServerNetworkHandler()->pack_id_to_content_key_.insert(content_keys.begin(),
                                                                                          content_keys.end());
    for (const auto &pack_instance : pack_stack->stack) {
        const auto &manifest = pack_instance.getManifest();
        const bool encrypted = content_keys.contains(manifest.getIdentity());
        getLogger().info("Loaded {} v{} (Pack ID: {}) {}", manifest.getName(),
                         manifest.getIdentity().version.asString(), manifest.getIdentity().id.asString(),
                         encrypted ? ColorFormat::Green + "[encrypted]" : "");
    }

    // Append loaded packs to level pack stack
    auto &level_stack = const_cast<ResourcePackStack &>(manager->getStack(ResourcePackStackType::LEVEL));
    level_stack.stack.insert(level_stack.stack.end(), std::make_move_iterator(pack_stack->stack.begin()),
                             std::make_move_iterator(pack_stack->stack.end()));
}

void EndstoneServer::registerEventListeners()
{
    auto &level = level_->getHandle();
    wrap<EndstoneActorGameplayHandler>(level.getActorEventCoordinator().actor_gameplay_handler_);
    wrap<EndstoneBlockGameplayHandler>(level.getBlockEventCoordinator().block_gameplay_handler_);
    wrap<EndstoneLevelGameplayHandler>(level.getLevelEventCoordinator().level_gameplay_handler_);
    wrap<EndstonePlayerGameplayHandler>(level.getServerPlayerEventCoordinator().player_gameplay_handler_);
    wrap<EndstoneScriptingEventHandler>(level.getScriptingEventCoordinator().scripting_event_handler_);
    wrap<EndstoneServerNetworkEventHandler>(level.getServerNetworkEventCoordinator().server_network_event_handler_);
}

void EndstoneServer::unregisterEventListeners()
{
    auto &level = level_->getHandle();
    unwrap<EndstoneActorGameplayHandler>(level.getActorEventCoordinator().actor_gameplay_handler_);
    unwrap<EndstoneBlockGameplayHandler>(level.getBlockEventCoordinator().block_gameplay_handler_);
    unwrap<EndstoneLevelGameplayHandler>(level.getLevelEventCoordinator().level_gameplay_handler_);
    unwrap<EndstonePlayerGameplayHandler>(level.getServerPlayerEventCoordinator().player_gameplay_handler_);
    unwrap<EndstoneScriptingEventHandler>(level.getScriptingEventCoordinator().scripting_event_handler_);
    unwrap<EndstoneServerNetworkEventHandler>(level.getServerNetworkEventCoordinator().server_network_event_handler_);
}

std::string EndstoneServer::getName() const
{
    return "Endstone";
}

std::string EndstoneServer::getVersion() const
{
    return ENDSTONE_VERSION;
}

std::string EndstoneServer::getMinecraftVersion() const
{
    static auto minecraft_version = [] {
        auto game_version = Common::getGameVersionString();
        if (game_version[0] == 'v') {
            game_version = game_version.substr(1);  // Removes the initial 'v'
        }
        return game_version;
    }();
    return minecraft_version;
}

Logger &EndstoneServer::getLogger() const
{
    return logger_;
}

Language &EndstoneServer::getLanguage() const
{
    return *language_;
}

EndstoneCommandMap &EndstoneServer::getCommandMap() const
{
    return *command_map_;
}

PluginManager &EndstoneServer::getPluginManager() const
{
    return *plugin_manager_;
}

PluginCommand *EndstoneServer::getPluginCommand(std::string name) const
{
    if (auto *command = command_map_->getCommand(name)) {
        return command->asPluginCommand();
    }
    return nullptr;
}

ConsoleCommandSender &EndstoneServer::getCommandSender() const
{
    return *command_sender_;
}

bool EndstoneServer::dispatchCommand(CommandSender &sender, std::string command_line) const
{
    return command_map_->dispatch(sender, std::move(command_line));
}

void EndstoneServer::loadPlugins()
{
    plugin_manager_->registerLoader(std::make_unique<CppPluginLoader>(*this));
    plugin_manager_->registerLoader(std::make_unique<PythonPluginLoader>(*this));

    auto plugin_dir = fs::current_path() / "plugins";

    if (exists(plugin_dir)) {
        plugin_manager_->loadPlugins(plugin_dir.string());
    }
    else {
        create_directories(plugin_dir);
    }
}

void EndstoneServer::enablePlugins(PluginLoadOrder type)
{
    if (type == PluginLoadOrder::PostWorld) {
        command_map_->setPluginCommands();
        DefaultPermissions::registerCorePermissions();
    }

    auto plugins = plugin_manager_->getPlugins();
    for (auto *plugin : plugins) {
        if (!plugin->isEnabled() && plugin->getDescription().getLoad() == type) {
            enablePlugin(*plugin);
        }
    }
}

void EndstoneServer::enablePlugin(Plugin &plugin)
{
    auto perms = plugin.getDescription().getPermissions();
    for (const auto &perm : perms) {
        if (plugin_manager_->addPermission(std::make_unique<Permission>(perm)) == nullptr) {
            getLogger().warning("Plugin {} tried to register permission '{}' that was already registered.",
                                plugin.getDescription().getFullName(), perm.getName());
        }
    }
    plugin_manager_->dirtyPermissibles(true);
    plugin_manager_->dirtyPermissibles(false);
    plugin_manager_->enablePlugin(plugin);
}

void EndstoneServer::disablePlugins() const
{
    plugin_manager_->disablePlugins();
}

Scheduler &EndstoneServer::getScheduler() const
{
    return *scheduler_;
}

Level *EndstoneServer::getLevel() const
{
    return level_.get();
}

std::vector<Player *> EndstoneServer::getOnlinePlayers() const
{
    std::vector<Player *> result;
    level_->getHandle().forEachPlayer([&](const ::Player &player) {
        result.emplace_back(&player.getEndstoneActor<EndstonePlayer>());
        return true;
    });
    return result;
}

int EndstoneServer::getMaxPlayers() const
{
    return getServer().getMinecraft()->getServerNetworkHandler()->getMaxNumPlayers();
}

Result<void> EndstoneServer::setMaxPlayers(int max_players)
{
    if (max_players < 0) {
        return nonstd::make_unexpected(make_error("Max number of players must not be negative."));
    }
    if (max_players > SharedConstants::NetworkDefaultMaxConnections) {
        return nonstd::make_unexpected(make_error("Max number of players must not exceed the hard limit {}",
                                                  SharedConstants::NetworkDefaultMaxConnections));
    }
    getServer().getMinecraft()->getServerNetworkHandler()->setMaxNumPlayers(max_players);
    return {};
}

Player *EndstoneServer::getPlayer(UUID id) const
{
    if (auto *player = level_->getHandle().getPlayer(EndstoneUUID::toMinecraft(id))) {
        return &player->getEndstoneActor<EndstonePlayer>();
    }
    return nullptr;
}

Player *EndstoneServer::getPlayer(std::string name) const
{
    for (const auto &player : getOnlinePlayers()) {
        if (boost::iequals(player->getName(), name)) {
            return player;
        }
    }
    return nullptr;
}

bool EndstoneServer::getOnlineMode() const
{
    return getServer().getMinecraft()->getServerNetworkHandler()->network_server_config_.require_trusted_authentication;
}

void EndstoneServer::shutdown()
{
    static_cast<EndstoneScheduler &>(getScheduler()).runTask([this]() {
        server_instance_->getMinecraft()->requestServerShutdown("");
    });
}

void EndstoneServer::reload()
{
    plugin_manager_->clearPlugins();
    command_map_->clearCommands();
    reloadData();

    // TODO(server): Wait for at most 2.5 seconds for all async tasks to finish, otherwise issue a warning
    loadPlugins();
    enablePlugins(PluginLoadOrder::Startup);
    enablePlugins(PluginLoadOrder::PostWorld);
    ServerLoadEvent event{ServerLoadEvent::LoadType::Reload};
    getPluginManager().callEvent(event);

    // sync commands
    for (const auto &player : getOnlinePlayers()) {
        player->updateCommands();
    }
}

void EndstoneServer::reloadData()
{
    unregisterEventListeners();
    server_instance_->getMinecraft()->requestResourceReload();
    level_->getHandle().loadFunctionManager();
    registerEventListeners();
}

void EndstoneServer::broadcast(const Message &message, const std::string &permission) const
{
    std::unordered_set<const CommandSender *> recipients;
    for (const auto *permissible : getPluginManager().getPermissionSubscriptions(permission)) {
        if (const auto *sender = permissible->asCommandSender(); sender && sender->hasPermission(permission)) {
            recipients.insert(sender);
        }
    }

    BroadcastMessageEvent event{!isPrimaryThread(), EndstoneMessage::toString(message), recipients};
    getPluginManager().callEvent(event);

    if (event.isCancelled()) {
        return;
    }

    for (const auto &recipient : recipients) {
        recipient->sendMessage(event.getMessage());
    }
}

void EndstoneServer::broadcastMessage(const Message &message) const
{
    broadcast(message, BroadcastChannelUser);
}

bool EndstoneServer::isPrimaryThread() const
{
    return std::this_thread::get_id() == server_instance_->server_instance_thread_.get_id();
}

Scoreboard *EndstoneServer::getScoreboard() const
{
    return scoreboard_.get();
}

std::shared_ptr<Scoreboard> EndstoneServer::createScoreboard()
{
    auto registry = CommandSoftEnumRegistry();
    auto board = std::make_unique<ServerScoreboard>(registry, nullptr, level_->getHandle().getGameplayUserManager());
    board->setPacketSender(level_->getHandle().getPacketSender());
    auto result = std::make_shared<EndstoneScoreboard>(std::move(board));
    return result;
}

float EndstoneServer::getCurrentMillisecondsPerTick()
{
    return current_mspt_;
}

float EndstoneServer::getAverageMillisecondsPerTick()
{
    return std::accumulate(average_mspt_, average_mspt_ + SharedConstants::TicksPerSecond, 0.0F) /
           SharedConstants::TicksPerSecond;
}

float EndstoneServer::getCurrentTicksPerSecond()
{
    return current_tps_;
}

float EndstoneServer::getAverageTicksPerSecond()
{
    return std::accumulate(average_tps_, average_tps_ + SharedConstants::TicksPerSecond, 0.0F) /
           SharedConstants::TicksPerSecond;
}

float EndstoneServer::getCurrentTickUsage()
{
    return current_usage_;
}

float EndstoneServer::getAverageTickUsage()
{
    return std::accumulate(average_usage_, average_usage_ + SharedConstants::TicksPerSecond, 0.0F) /
           SharedConstants::TicksPerSecond;
}

std::chrono::system_clock::time_point EndstoneServer::getStartTime()
{
    return start_time_;
}

std::unique_ptr<BossBar> EndstoneServer::createBossBar(std::string title, BarColor color, BarStyle style) const
{
    return std::make_unique<EndstoneBossBar>(std::move(title), color, style);
}

std::unique_ptr<BossBar> EndstoneServer::createBossBar(std::string title, BarColor color, BarStyle style,
                                                       std::vector<BarFlag> flags) const
{
    return std::make_unique<EndstoneBossBar>(std::move(title), color, style, flags);
}

Result<std::shared_ptr<BlockData>> EndstoneServer::createBlockData(std::string type) const
{
    return createBlockData(type, {});
}

Result<std::shared_ptr<BlockData>> EndstoneServer::createBlockData(std::string type, BlockStates block_states) const
{
    std::unordered_map<std::string, std::variant<int, std::string, bool>> states;
    for (const auto &state : block_states) {
        std::visit(overloaded{[&](auto &&arg) {
                       states.emplace(state.first, arg);
                   }},
                   state.second);
    }

    const auto block_descriptor = ScriptModuleMinecraft::ScriptBlockUtils::createBlockDescriptor(type, states);
    const auto *block = block_descriptor.tryGetBlockNoLogging();
    if (!block) {
        return nonstd::make_unexpected(make_error("Block type {} cannot be found in the registry.", type));
    }

    return std::make_shared<EndstoneBlockData>(const_cast<::Block &>(*block));
}

PlayerBanList &EndstoneServer::getBanList() const
{
    return *player_ban_list_;
}

IpBanList &EndstoneServer::getIpBanList() const
{
    return *ip_ban_list_;
}

ServiceManager &EndstoneServer::getServiceManager() const
{
    return *service_manager_;
}

EndstoneScoreboard &EndstoneServer::getPlayerBoard(const EndstonePlayer &player) const
{
    auto it = player_boards_.find(player.getUniqueId());
    if (it == player_boards_.end()) {
        return *scoreboard_;
    }
    return *it->second;
}

void EndstoneServer::setPlayerBoard(EndstonePlayer &player, Scoreboard &scoreboard)
{
    auto &old_board = getPlayerBoard(player).getHandle();
    auto &new_board = static_cast<EndstoneScoreboard &>(scoreboard).getHandle();

    if (&old_board == &new_board) {
        return;
    }

    // remove player from the old board
    getPlayerBoard(player).resetScores(&player);

    // add player to the new board
    new_board.onPlayerJoined(player.getPlayer());

    // update tracking records
    if (&scoreboard == scoreboard_.get()) {
        player_boards_.erase(player.getUniqueId());
    }
    else {
        player_boards_[player.getUniqueId()] =
            std::static_pointer_cast<EndstoneScoreboard>(scoreboard.shared_from_this());
    }
}

void EndstoneServer::removePlayerBoard(EndstonePlayer &player)
{
    player_boards_.erase(player.getUniqueId());
}

void EndstoneServer::tick(std::uint64_t current_tick, const std::function<void()> &tick_function)
{
    using namespace std::chrono;

    const auto tick_time = steady_clock::now();

    scheduler_->mainThreadHeartbeat(current_tick);
    tick_function();

    current_mspt_ = static_cast<float>(duration_cast<milliseconds>(steady_clock::now() - tick_time).count());
    current_tps_ =
        std::min(static_cast<float>(SharedConstants::TicksPerSecond), 1000.0F / std::max(1.0F, current_mspt_));
    current_usage_ = std::min(1.0F, current_mspt_ / SharedConstants::MilliSecondsPerTick);
    const auto idx = current_tick % SharedConstants::TicksPerSecond;
    average_mspt_[idx] = current_mspt_;
    average_tps_[idx] = current_tps_;
    average_usage_[idx] = current_usage_;
}

ServerInstance &EndstoneServer::getServer() const
{
    return *server_instance_;
}

}  // namespace endstone::core
