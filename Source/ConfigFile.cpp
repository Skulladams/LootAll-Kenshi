#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "Config.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
namespace LootAll {
std::wstring ModuleDirectory() {
    HMODULE module=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&ModuleDirectory),&module)) return L"";
    wchar_t path[32768]; DWORD size=GetModuleFileNameW(module,path,32768);
    if(!size||size>=32768) return L"";
    std::wstring full(path,size); return full.substr(0,full.find_last_of(L"\\/")+1);
}
Config LoadConfig() {
    std::ifstream file((ModuleDirectory()+L"LootAll.ini").c_str(),std::ios::binary);
    if(!file) { Log("LootAll.ini unavailable; using safe defaults."); return Config(); }
    std::ostringstream text; text<<file.rdbuf();
    std::vector<std::string> warnings; Config c=ParseConfig(ParseIni(text.str()),warnings);
    for(size_t i=0;i<warnings.size();++i) Log(warnings[i]);
    SetDebug(c.debug); return c;
}
}
