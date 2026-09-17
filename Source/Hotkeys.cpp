#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Hotkeys.h"
namespace LootAll {
static bool held[PresetCount]={false};
static bool Down(int key) { return key && (GetAsyncKeyState(key)&0x8000)!=0; }
bool GameHasFocus() {
    DWORD process=0; HWND window=GetForegroundWindow();
    if(!window) return false;
    GetWindowThreadProcessId(window,&process); return process==GetCurrentProcessId();
}
void PrimeHotkeys(const Config& c) { for(int i=0;i<PresetCount;++i) held[i]=Down(c.hotkeys[i].key); }
int PollHotkeys(const Config& c) {
    unsigned modifiers=(Down(VK_CONTROL)?1:0)|(Down(VK_MENU)?2:0)|(Down(VK_SHIFT)?4:0);
    int result=-1;
    for(int i=0;i<PresetCount;++i) {
        bool down=Down(c.hotkeys[i].key);
        if(down&&!held[i]&&modifiers==c.hotkeys[i].modifiers&&GameHasFocus()&&result<0) result=i;
        held[i]=down;
    }
    return result;
}
}
