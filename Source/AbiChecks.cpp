#include <string>
#include <cstddef>
#include <kenshi/Inventory.h>
#include <kenshi/Item.h>
#if !defined(_MSC_VER) || _MSC_VER != 1600 || !defined(_M_X64)
#error LootAll must use Visual C++ 2010 x64; a newer STL is ABI-incompatible with Kenshi.
#endif
static_assert(sizeof(void*)==8,"Kenshi is x64");
static_assert(sizeof(std::string)==40,"Kenshi uses release VC2010 string layout");
static_assert(sizeof(hand)==32,"Kenshi handle ABI mismatch");
static_assert(offsetof(InventorySection,width)==0x30,"InventorySection ABI mismatch");
static_assert(offsetof(InventoryItemBase,quantity)==0x12c,"Item quantity ABI mismatch");
