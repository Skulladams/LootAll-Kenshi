// LootAll - GPL-3.0-only. Shared, guarded KenshiLib hook initialization.
#pragma once
#include <core/Functions.h>
#include "Logger.h"
namespace LootAll {
bool ValidateGameStub(void* stub, const char* exportedName, const char* label);
bool ValidateGameTarget(void* target, const char* label);

template<typename Member, typename Function>
__forceinline bool InstallGameHook(const char* label, Member member,
    const char* exportedName, Function* detour, Function** original) {
    // Same conversion as KenshiLib's documented member-pointer overload.
    // VC2010 /GL + /LTCG is REQUIRED to resolve this through the KenshiLib IAT.
    static_assert(sizeof(Member)>=sizeof(void*), "Unsupported member-pointer ABI");
    void* stub=(void*&)member;
    if(!ValidateGameStub(stub,exportedName,label)) return false;
    intptr_t address=KenshiLib::GetRealAddress(stub);
    if(!ValidateGameTarget(reinterpret_cast<void*>(address),label)) return false;
    if(KenshiLib::AddHook(address,detour,original)!=KenshiLib::SUCCESS) {
        Log(std::string(label)+" installation failed; LootAll disabled");
        return false;
    }
    Log(std::string(label)+" installed");
    return true;
}
}
