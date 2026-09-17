#include "LootCandidate.h"
#include <algorithm>
namespace LootAll {
struct CandidateOrder {
    SortMode mode;
    explicit CandidateOrder(SortMode m):mode(m){}
    bool operator()(const LootCandidate& a,const LootCandidate& b) const {
        // Drain source bags before moving the bags themselves. Inner bags first.
        if(a.container!=b.container) return !a.container;
        if(a.container&&a.depth!=b.depth) return a.depth>b.depth;
        if(mode==OriginalOrder) return a.order<b.order;
        double left=mode==Value?a.value:a.efficiency, right=mode==Value?b.value:b.efficiency;
        if(left!=right) return left>right;
        if(a.gridEfficiency!=b.gridEfficiency) return a.gridEfficiency>b.gridEfficiency;
        if(a.value!=b.value) return a.value>b.value;
        return a.order<b.order;
    }
};
void SortCandidates(std::vector<LootCandidate>& candidates,const Config& c,Preset preset) {
    SortMode mode=(!c.bestItemsFirst?OriginalOrder:c.sort);
    if(preset==Valuable) mode=c.sort==OriginalOrder?ValuePerWeight:c.sort;
    std::stable_sort(candidates.begin(),candidates.end(),CandidateOrder(mode));
}
}
