# LootAll 1.0.1

LootAll is a convenience mod for Kenshi: open the selected player's inventory and press **Insert once** to loot nearby defeated enemies directly into available storage. It is not an AI job and does not make the character walk to bodies or open their inventories.

## Requirements and installation

- Kenshi x64, with a game version supported by RE_Kenshi.
- RE_Kenshi 0.3.5 or later with its matching KenshiLib and RVA files. The build was checked against the installed Kenshi 1.0.65 / KenshiLib 0.5.0 exports. Earlier combinations are not certified.
- Microsoft Visual C++ 2010 x64 runtime (normally already required by Kenshi).

Exit Kenshi, extract the `LootAll` folder from `LootAll-1.0.1.zip` into `Kenshi/mods`, then enable **LootAll** in the Kenshi launcher. The result must be `Kenshi/mods/LootAll/LootAll.dll`, with the INI, `.mod`, and JSON beside it. When upgrading, back up your edited INI and replace the old DLL; keep only one enabled LootAll installation. The configuration format is unchanged. Keep RE_Kenshi installed in the game root. Do not replace KenshiLib with the development libraries.

Version 1.0.1 fixes the confirmed 1.0.0 startup assertion caused by passing a local import thunk to KenshiLib. It enables the official VC2010 whole-program optimization settings, checks each hook pointer before resolution, and logs every installation step. All gameplay features and all four lifecycle/inventory hooks are preserved. The rebuilt startup still needs your in-game runtime test.

To uninstall, exit Kenshi, disable LootAll, and remove its folder. No persistent jobs or custom save records are added. Items already moved stay in their current inventories.

## Controls and presets

Select a conscious player character, open that character's main inventory, and press Insert. Keep that inventory open until the completion message. The backpack does not need its own window open. One press processes all eligible bodies; holding the key does not repeat.

| INI hotkey | Default | Action |
|---|---|---|
| LootNearby | INSERT | All enabled categories that pass value limits |
| TakeEverything | NONE | Ignores category and value filters, including the normal severed-limb category setting |
| ValuablePreset | NONE | Normal filters and limits, with value sorting forced on |
| DisarmPreset | NONE | Melee/ranged weapons and ammunition, subject to normal filters |
| SuppliesPreset | NONE | Food, medicine, repair kits, ammunition and tools, subject to normal filters |

Bindings accept letters, digits, F1–F24, NUMPAD0–NUMPAD9, INSERT, DELETE, HOME, END, PAGEUP, PAGEDOWN, SPACE, TAB, ENTER, ESCAPE, UP, DOWN, LEFT, RIGHT and PAUSE. Add CTRL, ALT and/or SHIFT, for example `CTRL+INSERT`. Modifiers must match exactly. `NONE` disables a command. Duplicate bindings disable the later command, with the main command taking priority. Choose keys that do not conflict with your other mods or game controls.

Restart Kenshi after editing hotkeys. Other gameplay settings are reread at the start of each activation. An operation uses a consistent configuration snapshot. Edit the UTF-8 INI beside the DLL; do not put comments after values on the same line. Malformed settings fall back to defaults.

## General and value settings

| Section / key | Default | Meaning |
|---|---|---|
| General / LootRadius | 20 | Meters, clamped to 1–100; 10 Kenshi world units per meter |
| PreferBackpack | 1 | Equipped backpack first, then main storage; 0 reverses the order |
| BestItemsFirst | 1 | Gather candidates from the entire battlefield before sorting |
| SortMode | ValuePerWeight | `Value`, `ValuePerWeight` or `OriginalOrder` |
| LootFriendlies | 0 | Only enemies. 1 also permits defeated allied/neutral NPCs; never player characters or the player faction |
| LootNearbyContainers | 0 | Reserved and unsupported in this body-looting release; 1 logs a warning and has no effect |
| AllowStolenItems | 1 | Preserve the engine's ownership/theft provenance. 0 conservatively excludes NPC body loot, including currently unmarked items that would acquire stolen provenance |
| DebugLogging | 0 | 1 adds scan counts and completion details to `RE_Kenshi_log.txt`; no idle/frame spam |
| Value / MinimumValue | 0 | Minimum engine player valuation in cats **per individual unit**, not whole-stack value |
| MinimumValuePerKg | 0 | Minimum per-unit value divided by unmodified per-unit weight |

Positive-value, zero-weight items have effectively unlimited value/kg. Zero-value, zero-weight items have zero efficiency. Invalid or negative item quantities/metrics are rejected. Sort ties use value per grid cell, then value and original order. No combinatorial packing is attempted. Container contents are considered before their containers; this is a structural exception to OriginalOrder.

## Categories

All `[Loot]` switches are 1 by default except `SeveredLimbs=0`. Classification uses Kenshi's `itemType`, `ItemFunction`, attachment slot, research-artifact flag, trade-item flag and loaded FCS reference lists; it does not guess categories from item names.

| Switch | Items |
|---|---|
| Weapons | Melee weapons and crossbows |
| Armour | Body armour, belts, gloves and neck equipment |
| Shirts / Pants / Boots / Headgear | Matching equipment slots; headgear includes eye attachments |
| Backpacks | Inventory containers/backpacks |
| Food | Normal and restricted food |
| Medicine | First aid and medical rigging |
| Ammo | Ammunition |
| BuildingMaterials | Items referenced by building `construction` lists |
| CraftingMaterials | Items in recipe `ingredients` or building `consumes` lists |
| Research | Research artifacts and items referenced by research `cost` lists, including research books |
| Blueprints | Blueprint item type/function |
| Books | Other items with the book function |
| Tools | Tool-function items |
| Robotics | Repair kits and loose replacement limbs |
| TradeGoods | Otherwise unclassified items flagged as trade items |
| Narcotics | Narcotic-function items |
| SeveredLimbs | Severed-limb-function items |
| Miscellaneous | Remaining items |

Specific equipment/consumable functions take precedence over recipe references. Research then books precede building materials, crafting materials, trade goods and miscellaneous. A mod-added item participates through its game data. Installed prosthetics and locked armour/restraints are protected because they require separate game actions.

## Blacklist and whitelist

Add any number of `Item1=...`, `Item2=...` entries under `[Blacklist]` and `[Whitelist]`. Prefer `id:<FCS stringID>`. `name:<exact display name>` is supported for convenience; matching is exact and case-sensitive. An unprefixed rule matches either an ID or display name. Names can change with localization.

Precedence: **validity, target/ownership/safety restrictions → blacklist → Take Everything → whitelist → preset/category/value filters**. A blacklist always wins. A whitelist bypasses category/value and preset selection, but cannot bypass protected targets, ownership policy, locked equipment, storage capacity, or safety checks. Take Everything also respects the blacklist and all global restrictions.

## Inventory and lifecycle behavior

Only dead or unconscious targets qualify. Conscious enemies, invalid entities, the player's faction and all player-controlled characters are excluded. Default friendly protection uses the game's character relationship checks, including temporary hostility, and explicitly protects allies.

Normal inventory, equipped slots and nested backpack contents are enumerated separately. The destination is storage only: LootAll does not equip loot onto the player. Kenshi checks dimensions, slot compatibility and stacking. Transfers are one unit at a time, allowing partial stacks to fill the backpack and continue in main storage. Rejected items remain on the source; smaller candidates are still tried when a large item does not fit. A backpack only moves when empty, so its filtered contents cannot silently bypass filters.

The queue stores game handles, resolves current inventories each step, and rechecks target state and range. Work is bounded to 32 unit attempts and about 4 ms per inventory update (an individual engine call can take longer). Scanning and classification indexing occur on activation, never continuously while idle. Closing/destroying the inventory, changing the selected character, unloading/loading the world, or losing the player cancels the queue. Invalid/recovered targets are skipped. Alt-tabbing pauses processing. No game pointers to source items or inventories are retained across updates.

Native `notifyTheftFrom` establishes theft provenance before insertion. Existing ownership is preserved; failed transactions restore their prior owner and exact source section. Quantity and stolen-unit accounting are checked after the engine accepts a unit. An ambiguous result disables further transfers and asks you to reload your pre-loot save. These checks are defensive safeguards, not a claim of runtime-proven inventory integrity on every game/mod combination.

## Feedback and troubleshooting

One native Kenshi message summarizes transferred **item units**, looted enemies, filtered units, space refusals and protected/unavailable units. A stack of 20 counts as 20 units. Debug details go to the existing RE_Kenshi log.

- No action: confirm the mod is enabled, RE_Kenshi is loaded, the selected character is conscious, and their main inventory is visible. Close modal dialogs and press/release the configured key.
- No targets: default mode protects neutral/allied NPCs; all targets must be dead/unconscious within the 3D radius. `AllowStolenItems=0` conservatively prevents body looting.
- Items left behind: inspect filters, backpack grid space, nonempty source bags, locks, installed prosthetics and whitelist/blacklist precedence. “Space exhausted” can mean no space for a particular shape; it does not mean every cell is occupied.
- Load failure: check `RE_Kenshi_log.txt` for `[LootAll] startPlugin entered`, per-hook `resolving`, `stub verified in KenshiLib.dll` and `installed` messages, ending with `[LootAll] initialization complete`. A rejected address is logged before calling `GetRealAddress`. Keep that log for diagnosis. Missing DLL/RVA or hook errors require a compatible RE_Kenshi installation. Kenshi's VC2010 C++ ABI is required.
- Hook conflicts: test without other automatic-looting plugins. BetterLooting was studied as a reference but is not a prerequisite.
- Invariant fault: stop, reload the pre-loot save, restart Kenshi and retain the log for diagnosis. Do not rely on later activations to repair an ambiguous engine operation.

There is no supported public RE_Kenshi settings-panel registration API in the inspected source. Configuration uses the complete INI rather than hooking undocumented settings internals. World-container theft, automatic jobs, auto-deposit behavior and installed-prosthetic removal are outside this release.

## Verification and short in-game test

See the adjacent `BUILD_REPORT.md` for compiler, test, PE and disassembly results. Version 1.0.0 was runtime-tested by the user and crashed at startup. The 1.0.1 fix has been built and statically verified; its startup and gameplay transfers are **not runtime-tested**. No game session or installed game files were modified during this fix.

1. Make a separate test save after a battle; record a corpse stack quantity and the player's matching stack quantity.
2. Install/enable the single 1.0.1 copy, restart, and confirm all four hook installations and the final `[LootAll] initialization complete` message with no hook errors.
3. Open the selected player's inventory and press Insert once among several downed enemies. Confirm one summary, multiple bodies looted, correct stack totals and theft labels; hold Insert to confirm no repeats.
4. Repeat with a nearly full backpack, then `PreferBackpack=0`; check both fallback directions and unchanged quantities on rejected items.
5. Verify neutral/allied and conscious NPCs are untouched, test a blacklist and a configured preset, then close/change inventory or load a save during a large operation to confirm cancellation.

## Source, credits and license

LootAll is GPL-3.0-only. It is independently structured, with adapted techniques from [BetterLooting](https://github.com/XxAtreuSSxX/BetterLooting), by XxAtreuSSxX. See `LICENSE` and `THIRD_PARTY_NOTICES.md` for attribution, licenses and pinned reference revisions. RE_Kenshi, KenshiLib and its examples are by BFrizzleFoShizzle and their respective contributors. Kenshi is by Lo-Fi Games; this project is unofficial.

The accompanying `LootAll-1.0.1-source.zip` contains corresponding LootAll source, build/test scripts, binary regression checks, disassembly evidence, an API provenance record and this configuration. Redistribute that archive alongside the binary ZIP, or provide corresponding source under GPLv3. Development dependencies are pinned in `DEPENDENCIES.md`; no compiler, game DLLs, or game data are redistributed in the installation ZIP.
