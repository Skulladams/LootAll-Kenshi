#pragma once
#include "Config.h"
#include <kenshi/util/hand.h>
#include <vector>
namespace LootAll {
struct LootCandidate {
    hand target,item; double value,efficiency,gridEfficiency; size_t order; bool container; int depth,remaining;
};
void SortCandidates(std::vector<LootCandidate>& candidates,const Config& c,Preset preset);
}
