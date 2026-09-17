#pragma once
#include <string>
namespace LootAll {
void Log(const std::string& message);
void Debug(const std::string& message);
void SetDebug(bool enabled);
void Notify(const std::string& message);
}
