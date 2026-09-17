#include "LootFilters.h"
#include <kenshi/Item.h>
#include <kenshi/GameData.h>
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
namespace LootAll {
static void References(GameData* data,const char* list,std::set<std::string>& result) {
    const Ogre::vector<GameDataReference>::type* refs=data->getReferenceListIfExists(list);
    if(refs) for(size_t i=0;i<refs->size();++i) result.insert((*refs)[i].sid);
}
void LootFilters::BuildIndex() {
    construction.clear(); crafting.clear(); research.clear();
    // FCS-declared reference lists, including mod-added records. No name heuristics.
    lektor<GameData*> buildings; ou->gamedata.getDataOfType(buildings,BUILDING);
    for(unsigned i=0;i<buildings.size();++i) if(buildings[i]) {
        References(buildings[i],"construction",construction);
        References(buildings[i],"consumes",crafting);
    }
    const itemType types[]={ITEM,ARMOUR,WEAPON,CROSSBOW,CONTAINER,LIMB_REPLACEMENT};
    for(int t=0;t<6;++t) {
        lektor<GameData*> records; ou->gamedata.getDataOfType(records,types[t]);
        for(unsigned i=0;i<records.size();++i) if(records[i]) References(records[i],"ingredients",crafting);
    }
    lektor<GameData*> techs; ou->gamedata.getDataOfType(techs,RESEARCH);
    for(unsigned i=0;i<techs.size();++i) if(techs[i]) References(techs[i],"cost",research);
}
Category LootFilters::Classify(Item* item) const {
    const itemType type=item->getItemType(); const ItemFunction f=item->itemFunction;
    if(type==WEAPON||type==CROSSBOW||f==ITEM_WEAPON) return Weapons;
    if(type==CONTAINER||f==ITEM_CONTAINER||item->slotType==ATTACH_BACKPACK) return Backpacks;
    if(type==ARMOUR||f==ITEM_CLOTHING) {
        switch(item->slotType) {
        case ATTACH_SHIRT: return Shirts; case ATTACH_LEGS: return Pants;
        case ATTACH_BOOTS: return Boots; case ATTACH_HAT: case ATTACH_EYES: return Headgear;
        default: return Armour; // Includes belt, gloves and neck equipment.
        }
    }
    if(type==BLUEPRINT||f==ITEM_BLUEPRINT) return Blueprints;
    if(type==LIMB_REPLACEMENT||f==ITEM_ROBOTREPAIR) return Robotics;
    switch(f) {
    case ITEM_FOOD: case ITEM_FOOD_RESTRICTED: return Food;
    case ITEM_FIRSTAID: case ITEM_MEDRIGGING: return Medicine;
    case ITEM_AMMO: return Ammo; case ITEM_NARCOTIC: return Narcotics;
    case ITEM_SEVERED_LIMB: return SeveredLimbs; case ITEM_TOOL: return Tools;
    default: break;
    }
    const std::string& id=item->getGameData()->stringID;
    if(item->isResearchArtifact()||research.count(id)) return Research;
    if(f==ITEM_BOOK) return Books;
    if(construction.count(id)) return BuildingMaterials;
    if(crafting.count(id)) return CraftingMaterials;
    if(item->isTradeItem) return TradeGoods;
    return Miscellaneous;
}
bool LootFilters::Accept(Item* item,const Config& c,Preset preset) const {
    if(!item||!item->isValid()||!item->getGameData()||!item->getGameData()->isValid()||item->quantity<=0||item->quantity>100000) return false;
    Config boundsOnly;
    if(!PassValues(item->getValueSingle(true),item->getItemWeightSingle(),boundsOnly)) return false;
    // Locked restraints and attached artificial limbs require game-specific actions.
    if(item->isLockedArmour() || (item->isEquipped&&item->getItemType()==LIMB_REPLACEMENT)) return false;
    if(!c.allowStolen && (item->isStolen(true)||!item->getProperOwner().isNull())) return false;
    const std::string& id=item->getGameData()->stringID; std::string name=item->getName();
    if(MatchRule(c.blacklist,id,name)) return false;
    if(preset==Everything) return true;
    if(MatchRule(c.whitelist,id,name)) return true;
    Category category=Classify(item);
    if(!c.categories[category]) return false;
    if(preset==Disarm && category!=Weapons&&category!=Ammo) return false;
    if(preset==Supplies && category!=Food&&category!=Medicine&&category!=Ammo&&item->itemFunction!=ITEM_ROBOTREPAIR&&category!=Tools) return false;
    return PassValues(item->getValueSingle(true),item->getItemWeightSingle(),c);
}
}
