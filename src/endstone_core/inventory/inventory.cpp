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

#include "endstone/detail/inventory/inventory.h"

#include "endstone/detail/inventory/item_stack.h"

namespace endstone::detail {

EndstoneInventory::EndstoneInventory(Container &container) : container_(container) {}

int EndstoneInventory::getSize() const
{
    return container_.getContainerSize();
}

int EndstoneInventory::getMaxStackSize() const
{
    return container_.getMaxStackSize();
}

std::shared_ptr<ItemStack> EndstoneInventory::getItem(int index) const
{
    return EndstoneItemStack::fromMinecraft(const_cast<::ItemStack &>(container_.getItem(index)));
}

void EndstoneInventory::setItem(int index, std::shared_ptr<ItemStack> item)
{
    container_.setItemWithForceBalance(index, *EndstoneItemStack::toMinecraft(item), true);
}

void EndstoneInventory::addItem(ItemStack &item)
{
    container_.addItemWithForceBalance(*EndstoneItemStack::toMinecraft(item.shared_from_this()));
}

std::vector<std::shared_ptr<ItemStack>> EndstoneInventory::getContents() const
{
    const auto slots = container_.getSlots();
    std::vector<std::shared_ptr<ItemStack>> contents;
    for (const auto &slot : slots) {
        if (slot && !slot->isNull()) {
            contents.push_back(EndstoneItemStack::fromMinecraft(const_cast<::ItemStack &>(*slot)));
        }
        else {
            contents.push_back(nullptr);
        }
    }
    return contents;
}

int EndstoneInventory::first(ItemStack &item)
{
    return container_.findFirstSlotForItem(*EndstoneItemStack::toMinecraft(item.shared_from_this()));
}

bool EndstoneInventory::isEmpty() const
{
    return container_.isEmpty();
}

void EndstoneInventory::clear()
{
    container_.removeAllItems();
}

}  // namespace endstone::detail
