// Adapted concepts from BetterLooting (XxAtreuSSxX), GPL-3.0-only.
// Named SDK members replace all raw layout offsets.
#include "WorldScanner.h"
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
#include <kenshi/Character.h>
#include <kenshi/Faction.h>
#include <kenshi/FactionRelations.h>
#include <kenshi/Inventory.h>
#include <kenshi/Item.h>
#include <set>
#include <limits>
namespace LootAll {
bool WorldReady() { return ou && ou->initialized && !ou->isLoadingFromASaveGame(); }
bool Eligible(Character* target,Character* player,const Config& c) {
    if(!WorldReady()||!target||!player||!target->isValid()||!player->isValid()||target==player) return false;
    if(ou->getIsInKillList(target)||target->isPlayerCharacter()) return false;
    if(!target->isDead()&&!target->isUnconcious()) return false;
    Faction* faction=target->getFaction(); Faction* mine=player->getFaction();
    if(!faction||!mine||faction==mine||faction->isThePlayer()) return false;
    if(!c.lootFriendlies) {
        if(target->isAlly(player,false)||player->isAlly(target,false)) return false;
        if(!target->isEnemy(player,false)&&!player->isEnemy(target,false)) return false;
    }
    const float radius=static_cast<float>(c.radius*10.0); // BetterLooting's proven conversion.
    float distance=(target->getPosition()-player->getPosition()).squaredLength();
    return distance>=0 && distance<=radius*radius && target->getInventory()!=NULL;
}
std::vector<hand> Scan(Character* player,const Config& c,int& nearby) {
    std::vector<hand> result; nearby=0;
    if(!WorldReady()||!player) return result;
    lektor<RootObject*> objects;
    // Grow only when the result hits the query limit; avoid a fixed 256-body cap
    // and avoid asking the game to preallocate an enormous result buffer.
    int limit=256;
    for(;;) {
        objects.clear();
        ou->getObjectsWithinSphere(objects,player->getPosition(),static_cast<float>(c.radius*10.0),CHARACTER,limit,player);
        if(objects.size()<static_cast<unsigned>(limit)) break;
        if(limit>std::numeric_limits<int>::max()/2) break;
        limit*=2;
    }
    nearby=objects.size(); std::set<hand> seen;
    for(unsigned i=0;i<objects.size();++i) {
        RootObject* object=objects[i]; if(!object||!object->isValid()||ou->getIsInKillList(object)) continue;
        hand h=object->getHandle(); if(!h.isValid()||!seen.insert(h).second) continue;
        Character* ch=h.getCharacter(); if(Eligible(ch,player,c)) result.push_back(h);
    }
    return result;
}
static void Walk(Inventory* inventory,std::vector<ItemLocation>& out,int depth,std::set<Inventory*>& inventories,std::set<Item*>& items) {
    if(!inventory||depth>8||!inventories.insert(inventory).second) return;
    const lektor<InventorySection*>& sections=inventory->getAllSections();
    for(unsigned s=0;s<sections.size();++s) {
        InventorySection* section=sections[s]; if(!section) continue;
        const Ogre::vector<InventorySection::SectionItem>::type& entries=section->getItems();
        for(size_t k=0;k<entries.size();++k) {
            Item* item=entries[k].item;
            if(!item||!item->isValid()||!items.insert(item).second) continue;
            ItemLocation loc; loc.inventory=inventory; loc.section=section; loc.item=item;
            loc.x=entries[k].x; loc.y=entries[k].y; loc.depth=depth;
            out.push_back(loc);
            Inventory* nested=item->getInventory();
            if(nested&&nested!=inventory) Walk(nested,out,depth+1,inventories,items);
        }
    }
}
void Enumerate(Inventory* inventory,std::vector<ItemLocation>& out,int depth) {
    std::set<Inventory*> inventories; std::set<Item*> items; Walk(inventory,out,depth,inventories,items);
}
ItemLocation Locate(Character* target,const hand& handle) {
    if(!target||!handle.isValid()) return ItemLocation();
    Item* item=handle.getItem(); if(!item||!item->isValid()) return ItemLocation();
    std::vector<ItemLocation> all; Enumerate(target->getInventory(),all);
    for(size_t i=0;i<all.size();++i) if(all[i].item==item) return all[i];
    return ItemLocation();
}
Inventory* Backpack(Character* player) {
    if(!player) return NULL;
    Inventory* main=player->getInventory(); if(!main) return NULL;
    InventorySection* slot=main->getSectionOfType(ATTACH_BACKPACK); if(!slot) return NULL;
    const Ogre::vector<InventorySection::SectionItem>::type& entries=slot->getItems();
    for(size_t i=0;i<entries.size();++i) {
        Item* bag=entries[i].item;
        if(bag&&bag->isValid()&&bag->isEquipped&&bag->getInventory()!=main) return bag->getInventory();
    }
    return NULL;
}
int Count(Inventory* inventory,GameData* data,bool stolenOnly) {
    if(!inventory) return 0;
    int count=0;
    const lektor<InventorySection*>& sections=inventory->getAllSections();
    for(unsigned s=0;s<sections.size();++s) if(sections[s]) {
        const Ogre::vector<InventorySection::SectionItem>::type& items=sections[s]->getItems();
        for(size_t i=0;i<items.size();++i) {
            Item* item=items[i].item;
            if(!item||item->getGameData()!=data||(stolenOnly&&!item->isStolen(true))) continue;
            // This is called inside a transfer transaction. Avoid allocating a set
            // after detaching a unit, while still defending against duplicate entries.
            bool duplicate=false;
            for(unsigned previous=0;previous<=s&&!duplicate;++previous) if(sections[previous]) {
                const Ogre::vector<InventorySection::SectionItem>::type& earlier=sections[previous]->getItems();
                size_t end=previous==s?i:earlier.size();
                for(size_t k=0;k<end;++k) if(earlier[k].item==item) { duplicate=true; break; }
            }
            if(!duplicate&&item->quantity>0&&item->quantity<=100000) {
                if(count>std::numeric_limits<int>::max()-item->quantity) return std::numeric_limits<int>::max();
                count+=item->quantity;
            }
        }
    }
    return count;
}
}
