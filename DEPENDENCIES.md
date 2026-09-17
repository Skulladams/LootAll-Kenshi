# Build dependencies and provenance

Use Release x64, Visual C++ 2010 (`cl` 16.00, v100), **`/GL` compilation and `/LTCG` linking**, `/MD`, Unicode, `_ITERATOR_DEBUG_LEVEL=0`. Whole-program optimization is required for correct KenshiLib member-function pointer imports. `AbiChecks.cpp` rejects other compiler/STL layouts. Windows SDK 7.1 supplies Windows headers/import libraries. No absolute addresses, raw object offsets or vtable positions are used by runtime source; the compile-time layout assertions compare the SDK to its documented ABI.

References inspected at these exact commits:

| Repository | Commit |
|---|---|
| https://github.com/XxAtreuSSxX/BetterLooting | 1b746d37dee3a7e08a9c5dbc59e5c31c6728a506 |
| https://github.com/BFrizzleFoShizzle/RE_Kenshi | 35eda1338caed2510143ca14a2556175bd1ae86a |
| https://github.com/BFrizzleFoShizzle/KenshiLib_Examples | 548b3eaf779c1b2feb25416f1db757320d04ec6c |
| https://github.com/BFrizzleFoShizzle/KenshiLib_Examples_deps | b566d74bf3d74629cc2fb632a97595b8202993f1 |
| https://github.com/BFrizzleFoShizzle/KenshiLib (tag v0.5.0) | 11cd65d557dea76c6540239acf467e0e06e09cf7 |
| https://github.com/Lucius64/KenshiExtensionPlugin (tag v0.17.3) | 08bb145c9b360bbfd2117c57b9104c5d2c18624e |

Clone the dependencies repository with Git LFS and check out the pinned commit. Extract `boost_1_60_0/boost.zip` in that folder. The dependency root must contain `KenshiLib/Include`, `KenshiLib/Libraries` and `boost_1_60_0/boost`.

Official compiler source: [Microsoft Windows SDK 7.1 ISO](https://www.microsoft.com/en-us/download/details.aspx?id=8442). The build used the x64 SDK distribution's VC2010 compiler, 16.00.30319.01. Extracted compiler files are development-only and are not distributed with LootAll. A normal v100 installation is also supported.

Install Python 3.8+ and the pinned verification dependencies, then build using PowerShell. `Build.ps1` requires a matching installed game directory for mandatory static checks; it never executes game DLLs. Python packages can instead be installed into an isolated directory and supplied with `-BinaryToolsPath` (or `LOOTALL_BINARY_TOOLS`).

```powershell
python -m pip install -r ./requirements-verification.txt
./Build.ps1 -VCRoot 'C:\path\VC' -SDKRoot 'C:\path\Windows\v7.1' -Dependencies 'C:\path\KenshiLib_Examples_deps' -KenshiDirectory 'C:\path\Kenshi' -Python python
./Test.ps1 -VCRoot 'C:\path\VC' -SDKRoot 'C:\path\Windows\v7.1'
./VerifyBinary.ps1 -Dll './Build/LootAll.dll' -KenshiDirectory 'C:\path\Kenshi'
python ./Tests/HookAddressTests.py --dll ./Build/LootAll.dll --kenshilib 'C:\path\Kenshi\KenshiLib.dll'
./Package.ps1 -Dll ./Build/LootAll.dll
```

Equivalent environment variables: `LOOTALL_VC100`, `LOOTALL_SDK71`, `LOOTALL_DEPS`, `LOOTALL_KENSHI`. Build uses the native `VC/bin/amd64` compiler, `VC/include`, `VC/lib/amd64`, SDK `Include` and `Lib/x64`. KenshiLib, Ogre and MyGUI import libraries are explicit linker inputs. Boost auto-link is disabled; header-only Boost.System disables its deprecated global `throws` object to avoid duplicate definitions across translation units. The only remaining compiler warning originates in upstream MyGUI's `BaseLayout.h` (C4091).

`VerifyHookAddresses.py` requires pefile 2024.8.26 and Capstone 5.0.7. It rejects unsupported optimizer output rather than guessing argument provenance. It parses table bounds and exports from the supplied DLL, not hardcoded RVAs. The source-archive `Verification` directory records the delivered build; new evidence is emitted into the chosen build directory. The binary regression suite accepts `--old-dll path/to/1.0.0/LootAll.dll` for the additional original-crash regression check (8 checks with it, 7 without it). No old or mutated DLL is executed.

The published KenshiExtensionPlugin v0.17.3 release DLL inspected for comparison has SHA256 `74c028ae13c1d9584857e1f5c0945782c5c40989f9bbb4caa07541a1724c2e59`. Its source enables WholeProgramOptimization for Release x64 with v100; its actual disassembly uses KenshiLib IAT pointer loads before `GetRealAddress`. See `BUILD_REPORT.md` for observed call sites. It is a research reference, not a LootAll dependency or bundled component.

## API evidence

- BetterLooting `Source/BetterLooting.cpp`: `CollectBetterLootingRadiusTargets` and `IsBetterLootingInsideConfiguredRadius` establish `getObjectsWithinSphere` and radius × 10; `IsBetterLootingCharacterTarget` uses `isDead` / `isUnconcious`; `InventoryGUIUpdate_hook` establishes main-inventory update handling; `ExecuteLootCandidate` demonstrates one-unit `removeItemDontDestroy_returnsItem`, section `addItem(item,1)`, native `notifyTheftFrom`, accounting, fallback and exact-source rollback.
- SDK `kenshi/Inventory.h`: section `getItems`, named dimensions/equipment flags, engine room/placement probes, inventory enumeration, mutation callbacks and container interfaces. LootAll uses structured section entries rather than BetterLooting's raw-offset grid reads.
- SDK `kenshi/Item.h`: named `quantity`, `itemFunction`, `slotType`, dimensions, `isTradeItem`, `isResearchArtifact`, `getValueSingle`, `getItemWeightSingle`, `getProperOwner`, `isStolen`, `notifyTheftFrom` and nested `getInventory`.
- SDK `kenshi/util/hand.h` and `RootObjectBase.h`: serial-bearing handles, `isValid`, typed resolution and identity. `GameWorld.h`: kill-list test, loading state, native user messages and `_clearAndDestroyGameWorldStuff` lifecycle hook.
- SDK `Character.h`: `isEnemy`, `isAlly`, `isPlayerCharacter`, character position/inventory. `Faction.h`: player-faction exclusion. `PlayerInterface.h`: selected-character handle.
- SDK `GameData.h` and `GameDataManager.h`: stable string IDs, typed records and references. The locally installed game's `fcs.def` defines `construction` (building), `consumes` (building), `ingredients` (craftable records), and `cost` (research). These list names are actual FCS schema names, not guessed fields.
- SDK `gui/InventoryGUI.h`: `_NV_update`, `_NV_show`, `_DESTRUCTOR`, visibility and callback-character APIs. Hooks resolve symbols with KenshiLib, without hardcoded RVAs.
- RE_Kenshi `Plugins.cpp`: active mod `RE_Kenshi.json`, `Plugins` array, DLL path relative to mod folder and exact decorated `startPlugin` export. The binary `.mod` is the unchanged empty-data manifest from the official HelloWorld example, renamed LootAll.mod.
- KenshiLib examples `HelloWorld.vcxproj` and README: v100 x64 and release ABI. The BetterLooting project currently names a newer toolset; LootAll follows the official ABI requirement instead.

KenshiLib implementation source: https://github.com/BFrizzleFoShizzle/KenshiLib (upstream fork of KenshiReclaimer/KenshiLib). The SDK/dependencies are external prerequisites, not replaced or modified by LootAll.
