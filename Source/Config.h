// LootAll - GPL-3.0-only
#pragma once
#include <string>
#include <vector>
#include <map>
namespace LootAll {
enum Category { Weapons, Armour, Shirts, Pants, Boots, Headgear, Backpacks,
    Food, Medicine, Ammo, BuildingMaterials, CraftingMaterials, Research,
    Blueprints, Books, Tools, Robotics, TradeGoods, Narcotics, SeveredLimbs,
    Miscellaneous, CategoryCount };
extern const char* const CategoryNames[CategoryCount];
enum Preset { Nearby, Everything, Valuable, Disarm, Supplies, PresetCount };
enum SortMode { Value, ValuePerWeight, OriginalOrder };
struct KeyBinding { int key; unsigned modifiers; KeyBinding(int k=0, unsigned m=0):key(k),modifiers(m){} };
struct Config {
    double radius, minimumValue, minimumValuePerKg;
    bool preferBackpack, bestItemsFirst, lootFriendlies, allowStolen, debug;
    bool categories[CategoryCount];
    SortMode sort;
    KeyBinding hotkeys[PresetCount];
    std::vector<std::string> blacklist, whitelist;
    Config();
};
typedef std::map<std::string, std::map<std::string,std::string> > Ini;
std::string Trim(const std::string& s);
std::string Upper(std::string s);
Ini ParseIni(const std::string& text);
Config ParseConfig(const Ini& ini, std::vector<std::string>& warnings);
KeyBinding ParseKey(const std::string& text, bool& valid);
bool MatchRule(const std::vector<std::string>& rules, const std::string& id, const std::string& name);
double Efficiency(double value, double weight);
bool PassValues(double value, double weight, const Config& config);
Config LoadConfig();
std::wstring ModuleDirectory();
}
