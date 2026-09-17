#pragma once
#include "Config.h"
namespace LootAll {
void PrimeHotkeys(const Config& c);
int PollHotkeys(const Config& c);
bool GameHasFocus();
}
