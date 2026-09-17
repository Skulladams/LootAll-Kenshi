#include "Logger.h"
#include <Debug.h>
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
namespace LootAll {
static bool debugEnabled=false;
void SetDebug(bool enabled) { debugEnabled=enabled; }
void Log(const std::string& message) { ErrorLog(std::string("[LootAll] ")+message); }
void Debug(const std::string& message) { if(debugEnabled) Log(message); }
void Notify(const std::string& message) {
    Log(message);
    if(ou && ou->initialized && !ou->isLoadingFromASaveGame())
        ou->showPlayerAMessage_withLog(std::string("LootAll: ")+message,true);
}
}
