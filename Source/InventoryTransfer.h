#pragma once
#include "WorldScanner.h"
namespace LootAll {
enum TransferResult { Moved, NoSpace, Unsafe, TransferFault };
TransferResult TransferOne(const ItemLocation& source,Character* target,Character* player,const Config& config);
}
