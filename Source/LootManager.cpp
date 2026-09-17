#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "LootManager.h"
#include "WorldScanner.h"
#include "InventoryTransfer.h"
#include "Hotkeys.h"
#include "Logger.h"
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/Character.h>
#include <kenshi/Inventory.h>
#include <kenshi/gui/InventoryGUI.h>
#include <kenshi/gui/ForgottenGUI.h>
#include <sstream>
#include <algorithm>
namespace LootAll {
LootManager::LootManager():config(LoadConfig()),windowIdentity(NULL),preset(Nearby),cursor(0),transferred(0),filtered(0),noSpace(0),unsafe(0),nearby(0),eligible(0),active(false),faulted(false),lastStep(0) { PrimeHotkeys(config); }
void LootManager::Cancel(const char* reason) {
    if(active) Log(std::string("Cancelled: ")+reason);
    active=false; candidates.clear(); looted.clear(); player.setNull(); windowIdentity=NULL;
}
void LootManager::Fault(const char* reason) { Cancel(reason); faulted=true; Notify(std::string("Looting disabled: ")+reason+". Reload your pre-loot save and restart Kenshi."); }
void LootManager::WindowClosed(InventoryGUI* window) {
    if(windowIdentity==window) { Cancel("active inventory closed or destroyed"); PrimeHotkeys(config); }
}
void LootManager::WindowShown() { PrimeHotkeys(config); }
void LootManager::Begin(InventoryGUI* window,Preset requested) {
    config=LoadConfig(); PrimeHotkeys(config);
    Character* actor=window->getCallbackCharacter();
    if(!actor||!actor->isValid()||!actor->isPlayerCharacter()) return;
    player=actor->getHandle(); windowIdentity=window; preset=requested;
    cursor=0; transferred=filtered=noSpace=unsafe=nearby=eligible=0; looted.clear(); candidates.clear();
    filters.BuildIndex();
    std::vector<hand> targets=Scan(actor,config,nearby); eligible=static_cast<int>(targets.size());
    Log(std::string("Loot requested by player: ")+actor->getName());
    for(size_t t=0;t<targets.size();++t) {
        Character* ch=targets[t].getCharacter(); if(!Eligible(ch,actor,config)) continue;
        std::vector<ItemLocation> items; Enumerate(ch->getInventory(),items);
        for(size_t i=0;i<items.size();++i) {
            Item* item=items[i].item;
            int qty=item->quantity;
            if(!config.allowStolen||!filters.Accept(item,config,preset)) { if(qty>0&&qty<=100000) filtered+=qty; continue; }
            hand h=item->getHandle(); if(!h.isValid()) { unsafe+=qty; continue; }
            LootCandidate c; c.target=targets[t]; c.item=h; c.value=item->getValueSingle(true);
            double weight=item->getItemWeightSingle();
            if(c.value<0||weight<0||weight!=weight) { unsafe+=qty; continue; }
            c.efficiency=Efficiency(c.value,weight);
            double area=static_cast<double>(std::max(1,item->itemWidth))*std::max(1,item->itemHeight);
            c.gridEfficiency=c.value/area; c.order=candidates.size();
            c.container=item->getInventory()!=NULL; c.depth=items[i].depth; c.remaining=qty;
            candidates.push_back(c);
        }
    }
    SortCandidates(candidates,config,preset);
    std::ostringstream status; status<<"Radius: "<<config.radius<<"m; Nearby objects: "<<nearby<<"; Eligible defeated NPCs: "<<eligible<<"; Candidate stacks: "<<candidates.size(); Debug(status.str());
    active=true; lastStep=0;
    if(candidates.empty()) Finish();
}
void LootManager::Finish() {
    std::ostringstream summary;
    summary<<transferred<<" item units looted from "<<looted.size()<<" enemies";
    if(noSpace) summary<<". Inventory space exhausted for "<<noSpace<<" units; left behind";
    if(filtered) summary<<". Filtered: "<<filtered;
    if(unsafe) summary<<". Unavailable or protected: "<<unsafe;
    active=false; candidates.clear(); looted.clear(); player.setNull(); windowIdentity=NULL;
    Notify(summary.str()); Debug("Completed successfully");
}
void LootManager::Update(InventoryGUI* window) {
    if(!window||window->ownerInventory!=NULL) return;
    if(!WorldReady()) { Cancel("world unavailable or loading"); return; }
    if(!window->isVisible()) { WindowClosed(window); return; }
    Character* actor=window->getCallbackCharacter();
    if(!actor||!actor->isValid()||!actor->isPlayerCharacter()||ou->getIsInKillList(actor)||window->getInventory()!=actor->getInventory()) {
        if(windowIdentity==window) Cancel("inventory owner changed"); return;
    }
    if(actor->isDead()||actor->isUnconcious()) { Cancel("selected player cannot act"); PrimeHotkeys(config); return; }
    // Only the selected player's visible main inventory may consume a key edge.
    if(!ou->player||ou->player->selectedCharacter.getCharacter()!=actor) {
        if(windowIdentity==window) Cancel("selected player changed"); return;
    }
    if(active && (windowIdentity!=window||!player.isValid()||player.getCharacter()!=actor)) Cancel("active inventory changed");
    if(!GameHasFocus() || (gui&&(gui->isLoadingMessageVisible()||gui->hasModalMessage()))) { PrimeHotkeys(config); return; }
    int request=PollHotkeys(config);
    if(!active&&!faulted&&request>=0) Begin(window,static_cast<Preset>(request));
    if(!active||faulted) return;
    DWORD now=GetTickCount(); if(lastStep && now-lastStep<8) return; lastStep=now;
    int steps=0;
    while(active&&cursor<candidates.size()&&steps<32 && GetTickCount()-now<4) {
        ++steps;
        LootCandidate& candidate=candidates[cursor];
        Character* source=candidate.target.isValid()?candidate.target.getCharacter():NULL;
        if(!Eligible(source,actor,config)) { unsafe+=candidate.remaining; ++cursor; continue; }
        ItemLocation loc=Locate(source,candidate.item);
        if(!loc.item||!filters.Accept(loc.item,config,preset)) { unsafe+=candidate.remaining; ++cursor; continue; }
        TransferResult result=TransferOne(loc,source,actor,config);
        if(result==TransferFault) { Fault("inventory invariant failed"); return; }
        if(result==Moved) {
            ++transferred; looted.insert(candidate.target);
            if(--candidate.remaining<=0) ++cursor;
        } else {
            if(result==NoSpace) noSpace+=candidate.remaining; else unsafe+=candidate.remaining;
            ++cursor;
        }
    }
    if(active&&cursor==candidates.size()) Finish();
}
}
