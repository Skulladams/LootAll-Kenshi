// Unit detach/add and exact-source rollback derived from BetterLooting, GPLv3.
#include "InventoryTransfer.h"
#include "Logger.h"
#include <kenshi/Inventory.h>
#include <kenshi/Character.h>
#include <kenshi/Item.h>
#include <vector>
namespace LootAll {
struct Destination { Inventory* inventory; InventorySection* section; };
static void Placements(Inventory* inv,Item* item,std::vector<Destination>& out) {
    if(!inv) return;
    const lektor<InventorySection*>& sections=inv->getAllSections();
    for(unsigned s=0;s<sections.size();++s) {
        InventorySection* section=sections[s];
        if(!section||!section->getEnabled()||section->containerSlot||section->isAnEquippedItemSection||!section->isLimitedSlotCompatible(item)) continue;
        int x=0,y=0;
        if(!section->hasRoomForItem(item->getGameData(),1)&&!section->getValidInventoryPosition(item,x,y)) continue;
        Destination d; d.inventory=inv; d.section=section; out.push_back(d);
    }
}
TransferResult TransferOne(const ItemLocation& src,Character* target,Character* player,const Config& c) {
    Item* item=src.item;
    if(!item||!src.section||!src.inventory||!src.section->hasItem(item)||!Eligible(target,player,c)) return Unsafe;
    if(item->quantity<=0||item->quantity>100000||item->isLockedArmour()) return Unsafe;
    Inventory* contents=item->getInventory();
    if(contents && !contents->isEmpty()) return Unsafe;
    // Native body-looting establishes theft provenance; with stolen loot disabled,
    // exclude NPC-owned body loot before touching its inventory, as BetterLooting does.
    if(!c.allowStolen) return Unsafe;
    Inventory* main=player->getInventory(); Inventory* pack=Backpack(player);
    std::vector<Destination> destinations;
    Placements(c.preferBackpack?pack:main,item,destinations);
    if(pack!=main) Placements(c.preferBackpack?main:pack,item,destinations);
    if(destinations.empty()) return NoSpace;
    GameData* data=item->getGameData();
    int sourceBefore=Count(src.inventory,data);
    int quantityBefore=item->quantity;
    const hand ownerBefore=item->getProperOwner();
    Item* detached=src.inventory->removeItemDontDestroy_returnsItem(item,1,quantityBefore>1);
    if(!detached) return Unsafe;
    const int removed=sourceBefore-Count(src.inventory,data);
    if(detached->quantity!=1||removed!=1) {
        // No destination has been touched. Restore exactly what the engine detached.
        if(removed>0 && detached->quantity==removed) {
            src.section->addItem(detached,1); src.inventory->notifyModified();
        }
        Log("Transfer invariant failed during detach; further transfers disabled. Reload the pre-loot save.");
        return TransferFault;
    }
    if(!detached->isStolen(true)) detached->notifyTheftFrom(target);
    bool stolen=detached->isStolen(true);
    if(stolen) for(size_t i=0;i<destinations.size();++i) {
        Destination& dest=destinations[i];
        if(dest.inventory==src.inventory) continue;
        int before=Count(dest.inventory,data), stolenBefore=Count(dest.inventory,data,true);
        bool accepted=dest.section->addItem(detached,1);
        dest.inventory->notifyModified(); src.inventory->notifyModified();
        int delta=Count(dest.inventory,data)-before;
        if(delta==1 && sourceBefore-Count(src.inventory,data)==1) {
            // The engine can merge and destroy the detached pointer; never read it here.
            if(Count(dest.inventory,data,true)-stolenBefore!=1) {
                Log("Ownership invariant failed; further transfers disabled. Reload the pre-loot save."); return TransferFault;
            }
            return Moved;
        }
        // Only a definite refusal permits retry/rollback. Do not touch a possibly merged pointer.
        if(accepted||delta!=0) {
            Log("Ambiguous engine transfer result; further transfers disabled. Reload the pre-loot save."); return TransferFault;
        }
    }
    // Restore the exact prior owner only on the failed transaction's detached unit.
    // Successful transfers never clear ownership or stolen flags.
    detached->setProperOwner(ownerBefore);
    bool restored=src.section->addItem(detached,1);
    src.inventory->notifyModified();
    if(!restored||Count(src.inventory,data)!=sourceBefore) {
        Log("Exact-source rollback failed; further transfers disabled. Reload the pre-loot save."); return TransferFault;
    }
    return stolen?NoSpace:Unsafe;
}
}
