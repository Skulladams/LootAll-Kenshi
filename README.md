# LootAll for Kenshi

LootAll is a native RE_Kenshi plugin that makes looting large groups of defeated enemies much easier.

Open your character's inventory, press **Insert**, and LootAll will scan nearby dead or unconscious NPCs and transfer eligible loot directly into your available inventory space.
 
**No walking from corpse to corpse.**  
**No opening every body individually.**

## Why I Made This

I came back to Kenshi after about a 3-year hiatus and found that the BetterLooting mod I used was no longer available on the Steam Workshop.

After looking around for a backup and seeing people discussing recreating it, I decided to build a replacement for my own game using GPT/Codex.

I got it working and tested it in-game, so I figured I might as well share it with anyone else looking for the same functionality.

LootAll is heavily inspired by **BetterLooting** and uses its open-source implementation as an important technical reference.

## Requirements

- Kenshi x64
- RE_Kenshi 0.3.5 or newer
- Compatible KenshiLib / RVA files
- Microsoft Visual C++ 2010 x64 runtime

### Tested With

- Kenshi 1.0.65 x64
- RE_Kenshi 0.3.5
- KenshiLib 0.5.0

## Installation

1. Go to the **Releases** section of this repository.
2. Download `LootAll-1.0.1.zip`.
3. Close Kenshi completely.
4. Extract the included `LootAll` folder into:

```text
Kenshi/mods/
```

Your install should look like:

```text
Kenshi/
└── mods/
    └── LootAll/
        ├── LootAll.dll
        ├── LootAll.ini
        ├── LootAll.mod
        ├── RE_Kenshi.json
        ├── README.md
        ├── LICENSE
        └── THIRD_PARTY_NOTICES.md
```

5. Start Kenshi.
6. Enable **LootAll** in the Kenshi launcher.
7. Make sure RE_Kenshi is installed and working.

## How to Use

1. Select the character that should receive the loot.
2. Open that character's **main inventory**.
3. Stand near dead or unconscious enemies.
4. Press **Insert once**.

LootAll will scan nearby defeated NPCs and begin transferring eligible items.

You only need to press **Insert once**.

Your character does not physically walk to each body.

## Default Looting Range

The default radius is:

**20 meters**

You can change it in `LootAll.ini`.

Supported range:

**1-100 meters**

Example:

```ini
[General]
LootRadius=30
```

## Inventory Behavior

By default LootAll prefers your equipped backpack.

The transfer order is:

1. Backpack
2. Main inventory

If an item cannot fit in either inventory, it is left on the original NPC.

LootAll is designed to avoid deleting or duplicating items when a transfer fails.

The setting is:

```ini
[General]
PreferBackpack=1
```

Set it to `0` if you want LootAll to try the main inventory first.

## Loot Filters

LootAll can filter many different item categories.

Default categories include:

- Weapons
- Armour
- Shirts
- Pants
- Boots
- Headgear
- Backpacks
- Food
- Medicine
- Ammo
- Building materials
- Crafting materials
- Research items
- Blueprints
- Books
- Tools
- Robotics
- Trade goods
- Narcotics
- Miscellaneous items

Severed limbs are disabled by default.

Example:

```ini
[Loot]
Weapons=1
Armour=1
Shirts=1
Pants=1
Boots=1
Headgear=1
Backpacks=1
Food=1
Medicine=1
Ammo=1
BuildingMaterials=1
CraftingMaterials=1
Research=1
Blueprints=1
Books=1
Tools=1
Robotics=1
TradeGoods=1
Narcotics=1
SeveredLimbs=0
Miscellaneous=1
```

`1` means enabled.

`0` means disabled.

## Valuable Loot and Sorting

By default LootAll prioritizes useful loot using value compared with weight.

Default settings:

```ini
[General]
BestItemsFirst=1
SortMode=ValuePerWeight
```

Available sorting modes:

```text
ValuePerWeight
Value
OriginalOrder
```

### ValuePerWeight

Prioritizes items that are worth more Cats relative to their weight.

### Value

Prioritizes the highest raw item value.

### OriginalOrder

Processes items in the order they were found.

## Minimum Value Filters

You can prevent LootAll from taking cheap items.

Example:

```ini
[Value]
MinimumValue=0
MinimumValuePerKg=0
```

`MinimumValue` sets the minimum item value in Cats.

`MinimumValuePerKg` sets the minimum value compared with item weight.

Leaving both at `0` disables value filtering.

## Additional Loot Modes

LootAll also supports optional preset hotkeys.

These are disabled by default.

### Take Everything

Attempts to take everything from eligible defeated enemies while still respecting available inventory space and safety checks.

### Valuable

Uses your value filters to focus on valuable items.

### Disarm

Focuses on:

- Weapons
- Ranged weapons
- Ammunition

Useful for quickly stripping weapons from defeated or unconscious enemies.

### Supplies

Focuses on useful supplies such as:

- Food
- Medicine
- Robotics repair items
- Ammunition
- Tools and survival supplies

Default configuration:

```ini
[Hotkeys]
LootNearby=INSERT
TakeEverything=NONE
ValuablePreset=NONE
DisarmPreset=NONE
SuppliesPreset=NONE
```

You can assign additional hotkeys in `LootAll.ini`.

## Stolen Items

LootAll does **not** remove or bypass Kenshi's normal stolen-item and ownership mechanics.

If Kenshi considers an item stolen, LootAll preserves that status.

Default:

```ini
[General]
AllowStolenItems=1
```

Set this to:

```ini
AllowStolenItems=0
```

if you want LootAll to skip items already considered stolen.

## Friendly NPC Protection

By default LootAll does not loot friendly or allied NPCs.

```ini
[General]
LootFriendlies=0
```

Player-controlled characters and members of the player's faction are protected.

## Nearby Containers

LootAll includes optional support for nearby containers where safely supported.

This is disabled by default.

```ini
[General]
LootNearbyContainers=0
```

Body looting is the main purpose of the mod.

## Whitelist and Blacklist

LootAll supports optional item whitelist and blacklist rules.

Example:

```ini
[Blacklist]
Item1=Iron Stick
Item2=Rag Shirt

[Whitelist]
Item1=Ancient Science Book
Item2=Engineering Research
```

Blacklisted items will be skipped.

Whitelisted items can be used to prioritize specific items regardless of normal value/category filtering where supported.

## Configuration

The main configuration file is:

```text
LootAll.ini
```

Default settings look similar to:

```ini
[General]
LootRadius=20
PreferBackpack=1
BestItemsFirst=1
SortMode=ValuePerWeight
LootFriendlies=0
LootNearbyContainers=0
AllowStolenItems=1
DebugLogging=0

[Hotkeys]
LootNearby=INSERT
TakeEverything=NONE
ValuablePreset=NONE
DisarmPreset=NONE
SuppliesPreset=NONE

[Loot]
Weapons=1
Armour=1
Shirts=1
Pants=1
Boots=1
Headgear=1
Backpacks=1
Food=1
Medicine=1
Ammo=1
BuildingMaterials=1
CraftingMaterials=1
Research=1
Blueprints=1
Books=1
Tools=1
Robotics=1
TradeGoods=1
Narcotics=1
SeveredLimbs=0
Miscellaneous=1

[Value]
MinimumValue=0
MinimumValuePerKg=0
```

Most gameplay settings are loaded from the configuration file.

Hotkey changes may require restarting Kenshi.

## Debug Logging

Debug logging is disabled by default.

```ini
DebugLogging=0
```

If you experience a problem, change it to:

```ini
DebugLogging=1
```

Then reproduce the issue and check the RE_Kenshi / LootAll logs.

## Uninstall

1. Close Kenshi.
2. Disable LootAll in the launcher or remove:

```text
Kenshi/mods/LootAll/
```

LootAll does not create a permanent AI looting job or separate save system.

Items already looted will remain wherever they currently are.

## Source Code

The complete source code is included in this repository.

A source archive is also provided with each release.

LootAll is written as a native C++ RE_Kenshi / KenshiLib plugin.

## Building

The project targets:

- Windows x64
- Release configuration
- Visual C++ 2010 compatible toolchain
- RE_Kenshi
- KenshiLib

See:

```text
BUILD_REPORT.md
DEPENDENCIES.md
```

for additional build and dependency information.

## Credits

### BetterLooting

LootAll was heavily inspired by and technically informed by the original **BetterLooting** project.

Original BetterLooting repository:

https://github.com/XxAtreuSSxX/BetterLooting

BetterLooting was created by **XxAtreuSSxX and contributors** and is distributed under the GNU General Public License version 3.

LootAll adapts techniques demonstrated by BetterLooting, including concepts related to:

- Nearby target scanning
- Dead/unconscious character checks
- Inventory enumeration
- Inventory transfers
- Backpack/main inventory handling
- Loot filtering
- Theft and ownership handling
- Battlefield mass looting

This project is **not** the original BetterLooting mod and is **not presented as an official continuation by the original author**.

The goal is to provide similar functionality for players after the original Workshop release became unavailable.

### RE_Kenshi

LootAll uses RE_Kenshi.

https://github.com/BFrizzleFoShizzle/RE_Kenshi

Thanks to BFrizzleFoShizzle, KenshiReclaimer, and the other contributors who made native Kenshi modding possible.

## AI / Codex Disclosure

LootAll was developed with  assistance from GPT/Codex.

AI was used to help inspect the available open-source references, write and organize code, troubleshoot compilation/runtime problems.

The mod was also tested in-game before being published.

## License

LootAll is released under the **GNU General Public License version 3 (GPLv3)**.

See:

- `LICENSE`
- `THIRD_PARTY_NOTICES.md`

for complete licensing and attribution information.

## Disclaimer

Kenshi is developed by Lo-Fi Games.

LootAll is an unofficial community mod and is not affiliated with or endorsed by Lo-Fi Games, the original BetterLooting author, RE_Kenshi, or KenshiLib contributors.

Use mods at your own risk and keep backups of important saves.
