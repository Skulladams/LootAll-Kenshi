#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <stdint.h>
#include <cstdio>
#include "HookInstallation.h"
namespace LootAll {
static HMODULE ModuleAt(void* address) {
    HMODULE module=NULL;
    if(!address || !GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(address),&module)) return NULL;
    return module;
}
// Do not follow or execute a suspect import thunk. Check its actual module,
// exact exported identity and the real exported table boundaries first.
__declspec(noinline) bool ValidateGameStub(void* stub,const char* exportedName,const char* label) {
    Log(std::string("resolving ")+label);
    HMODULE library=GetModuleHandleW(L"KenshiLib.dll");
    FARPROC expected=library?GetProcAddress(library,exportedName):NULL;
    FARPROC begin=library?GetProcAddress(library,"FUNC_BEGIN"):NULL;
    FARPROC end=library?GetProcAddress(library,"FUNC_END"):NULL;
    const uintptr_t address=reinterpret_cast<uintptr_t>(stub);
    if(!library || !expected || !begin || !end || ModuleAt(stub)!=library ||
        stub!=reinterpret_cast<void*>(expected) ||
        address<reinterpret_cast<uintptr_t>(begin) || address>=reinterpret_cast<uintptr_t>(end)) {
        char message[512];
        sprintf_s(message,"%s rejected: stub=%p expected=%p KenshiLib=%p table=[%p,%p). Check /GL + /LTCG; GetRealAddress was NOT called.",
            label,stub,reinterpret_cast<void*>(expected),library,
            reinterpret_cast<void*>(begin),reinterpret_cast<void*>(end));
        Log(message);
        return false;
    }
    char message[256];
    sprintf_s(message,"%s stub verified in KenshiLib.dll: %p",label,stub);
    Log(message);
    return true;
}
__declspec(noinline) bool ValidateGameTarget(void* target,const char* label) {
    // These four hooks all target engine functions, not library implementations.
    if(ModuleAt(target)!=GetModuleHandleW(NULL)) {
        Log(std::string(label)+" resolved outside Kenshi.exe; hook rejected");
        return false;
    }
    return true;
}
}
