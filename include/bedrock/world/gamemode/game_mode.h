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
#include "bedrock/forward.h"
#include "bedrock/gameplayhandlers/coordinator_result.h"
#include "bedrock/world/item/item_stack.h"
#include "bedrock/world/level/block/block_legacy.h"
#include "bedrock/world/level/block_pos.h"

class Player;

class GameMode {
public:
    virtual ~GameMode() = 0;
    virtual bool startDestroyBlock(BlockPos const &, FacingID face, bool &) = 0;
    ENDSTONE_HOOK virtual bool destroyBlock(BlockPos const &, FacingID);
    virtual bool continueDestroyBlock(BlockPos const &, FacingID, Vec3 const &, bool &) = 0;
    virtual void stopDestroyBlock(BlockPos const &) = 0;
    virtual void startBuildBlock(BlockPos const &, FacingID) = 0;
    virtual bool buildBlock(BlockPos const &, FacingID, bool) = 0;
    virtual void continueBuildBlock(BlockPos const &, FacingID) = 0;
    virtual void stopBuildBlock() = 0;
    virtual void tick() = 0;
    virtual float getPickRange(InputMode const &, bool) = 0;
    virtual bool useItem(ItemStack &) = 0;
    ENDSTONE_HOOK virtual InteractionResult useItemOn(ItemStack &, BlockPos const &, FacingID, Vec3 const &,
                                                      Block const *, bool);
    ENDSTONE_HOOK virtual bool interact(Actor &, Vec3 const &);
    virtual bool attack(Actor &) = 0;
    virtual void releaseUsingItem() = 0;
    virtual void setTrialMode(bool) = 0;
    virtual bool isInTrialMode() = 0;
    virtual void registerUpsellScreenCallback(std::function<void(bool)>) = 0;

private:
    Player *player_;
};
