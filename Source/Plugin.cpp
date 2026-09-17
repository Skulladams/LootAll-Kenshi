// Copyright (C) 2026 LootAll contributors. GPL-3.0-only.
#include "LootManager.h"
#include "Logger.h"
#include "HookInstallation.h"
#include <kenshi/gui/InventoryGUI.h>
#include <kenshi/GameWorld.h>
#include <exception>
namespace {
LootAll::LootManager* manager=NULL;
void (*updateOriginal)(InventoryGUI*)=NULL;
void (*showOriginal)(InventoryGUI*,bool)=NULL;
void (*destroyOriginal)(InventoryGUI*)=NULL;
void (*clearWorldOriginal)(GameWorld*)=NULL;
void Update(InventoryGUI* self) {
    updateOriginal(self);
    if(!manager) return;
    try { manager->Update(self); }
    catch(const std::exception& e) { manager->Fault(e.what()); }
    catch(...) { manager->Fault("unexpected exception"); }
}
void Show(InventoryGUI* self,bool on) {
    if(!on&&manager) manager->WindowClosed(self);
    showOriginal(self,on);
    if(on&&manager) manager->WindowShown();
}
void Destroy(InventoryGUI* self) {
    if(manager) manager->WindowClosed(self);
    destroyOriginal(self);
}
void ClearWorld(GameWorld* self) {
    if(manager) manager->Cancel("saved game unloading or world shutting down");
    clearWorldOriginal(self);
}
}
// RE_Kenshi resolves the C++ decorated export ?startPlugin@@YAXXZ.
__declspec(dllexport) void startPlugin() {
    LootAll::Log("startPlugin entered");
    LootAll::Log("Plugin loaded, version 1.0.1 (MSVC2010 x64, /GL + /LTCG)");
    try {
        if(!LootAll::InstallGameHook("GameWorld lifecycle hook",&GameWorld::_clearAndDestroyGameWorldStuff,
            "?_clearAndDestroyGameWorldStuff@GameWorld@@QEAAXXZ",ClearWorld,&clearWorldOriginal)) return;
        if(!LootAll::InstallGameHook("InventoryGUI show hook",&InventoryGUI::_NV_show,
            "?_NV_show@InventoryGUI@@QEAAX_N@Z",Show,&showOriginal)) return;
        if(!LootAll::InstallGameHook("InventoryGUI destructor hook",&InventoryGUI::_DESTRUCTOR,
            "?_DESTRUCTOR@InventoryGUI@@QEAAXXZ",Destroy,&destroyOriginal)) return;
        if(!LootAll::InstallGameHook("InventoryGUI update hook",&InventoryGUI::_NV_update,
            "?_NV_update@InventoryGUI@@QEAAXXZ",Update,&updateOriginal)) return;
        manager=new LootAll::LootManager();
        LootAll::Log("Config loaded; hotkeys initialized. Open the selected player's inventory and press Insert.");
        LootAll::Log("initialization complete");
    } catch(const std::exception& e) {
        LootAll::Log(std::string("initialization failed: ")+e.what());
    } catch(...) {
        LootAll::Log("initialization failed: unexpected C++ exception");
    }
}
