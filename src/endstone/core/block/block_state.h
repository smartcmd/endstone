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

#include "endstone/block/block.h"
#include "endstone/block/block_state.h"
#include "endstone/core/block/block.h"
#include "endstone/core/level/dimension.h"

namespace endstone::core {

class EndstoneBlockState : public BlockState {
public:
    explicit EndstoneBlockState(const EndstoneBlock &block);
    explicit EndstoneBlockState(Dimension &dimension, BlockPos block_pos, const ::Block &block);
    [[nodiscard]] std::unique_ptr<Block> getBlock() const override;
    [[nodiscard]] std::string getType() const override;
    Result<void> setType(std::string type) override;
    [[nodiscard]] std::unique_ptr<BlockData> getData() const override;
    Result<void> setData(const BlockData &data) override;
    [[nodiscard]] Dimension &getDimension() const override;
    [[nodiscard]] int getX() const override;
    [[nodiscard]] int getY() const override;
    [[nodiscard]] int getZ() const override;
    [[nodiscard]] Location getLocation() const override;
    bool update() override;
    bool update(bool force) override;
    bool update(bool force, bool apply_physics) override;

private:
    EndstoneDimension &dimension_;
    BlockSource &block_source_;
    BlockPos block_pos_;
    ::Block *block_;
};

}  // namespace endstone::core
