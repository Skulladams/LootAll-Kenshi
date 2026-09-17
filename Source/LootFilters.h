#pragma once
#include "Config.h"
#include <set>
class Item;
class GameData;
namespace LootAll {
class LootFilters {
    std::set<std::string> construction, crafting, research;
public:
    void BuildIndex();
    Category Classify(Item* item) const;
    bool Accept(Item* item, const Config& config, Preset preset) const;
};
}
