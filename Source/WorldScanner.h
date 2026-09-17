#pragma once
#include "Config.h"
#include <kenshi/util/hand.h>
#include <vector>
class Character;
class Inventory;
class InventorySection;
class Item;
namespace LootAll {
struct ItemLocation {
    Inventory* inventory; InventorySection* section; Item* item; int x,y,depth;
    ItemLocation():inventory(NULL),section(NULL),item(NULL),x(0),y(0),depth(0){}
};
bool WorldReady();
bool Eligible(Character* target, Character* player, const Config& config);
std::vector<hand> Scan(Character* player,const Config& config,int& nearby);
void Enumerate(Inventory* inventory,std::vector<ItemLocation>& out,int depth=0);
ItemLocation Locate(Character* target, const hand& item);
Inventory* Backpack(Character* player);
int Count(Inventory* inventory,GameData* data,bool stolenOnly=false);
}
