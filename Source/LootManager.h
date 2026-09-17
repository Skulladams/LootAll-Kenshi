#pragma once
#include "LootCandidate.h"
#include "LootFilters.h"
#include <set>
class InventoryGUI;
namespace LootAll {
class LootManager {
    Config config;
    LootFilters filters;
    std::vector<LootCandidate> candidates;
    std::set<hand> looted;
    hand player;
    InventoryGUI* windowIdentity; // Compared only; never dereferenced across updates.
    Preset preset;
    size_t cursor;
    int transferred,filtered,noSpace,unsafe,nearby,eligible;
    bool active,faulted;
    unsigned long lastStep;
    void Begin(InventoryGUI* window,Preset requested);
    void Finish();
public:
    LootManager();
    void Update(InventoryGUI* window);
    void Cancel(const char* reason);
    void WindowClosed(InventoryGUI* window);
    void WindowShown();
    void Fault(const char* reason);
};
}
