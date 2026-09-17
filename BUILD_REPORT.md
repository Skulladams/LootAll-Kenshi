# LootAll 1.0.1 build and startup-crash verification

September 17, 2026. Target: Kenshi 1.0.65 x64 Steam, RE_Kenshi 0.3.5, KenshiLib 0.5.0.

**Rebuilt successfully with VC2010 x64. Startup still requires an in-game test by the user.** The user's confirmed 1.0.0 crash is the baseline for this fix; it is not described as a successful runtime build.

## Root cause and correction

The original build omitted whole-program optimization. SDK game member functions are declared without `dllimport`; taking their address in an ordinary VC2010 object can produce a linker-created jump thunk inside LootAll.dll. The old lifecycle resolver loads that local thunk with `LEA RCX` at RVA `0x1147A`, then calls `GetRealAddress` at `0x11481`. KenshiLib expects its own generated stub, so the release assertion fires before gameplay begins.

KenshiLib's exact [v0.5.0 implementation](https://github.com/BFrizzleFoShizzle/KenshiLib/blob/11cd65d557dea76c6540239acf467e0e06e09cf7/Source/core/Functions.cpp) diagnoses this caller-module case and explicitly recommends whole-program optimization. The official examples, RE_Kenshi Release x64 project, BetterLooting project and KenshiExtensionPlugin Release x64 project enable it. LootAll now compiles with `/GL` and links with `/LTCG`, using the same VC2010 toolchain and import libraries. A configuration-only intermediate build proved that these flags alone correct all four argument loads.

The published [KenshiExtensionPlugin v0.17.3](https://github.com/Lucius64/KenshiExtensionPlugin/releases/tag/v0.17.3) targets KenshiLib 0.5.0 / RE_Kenshi 0.3.5. Its source and actual release binary were inspected. In that binary, `startPlugin` at RVA `0x5272B` loads the `TownBase::_NV_setFaction` IAT pointer before the getter call at `0x52732`; RVA `0x527C8` similarly loads `Inventory::getEquippedArmour` before the getter call at `0x527CF`. This confirms the same import-load method in a published compatible plugin. That reference plugin was not launched in the user's game during this task.

`InstallGameHook` centralizes all four LootAll resolutions. Before every `GetRealAddress` call, its guard uses Windows module/export lookup to require that the pointer belongs to KenshiLib.dll, equals the exact named exported stub, and lies between its actual exported `FUNC_BEGIN` and `FUNC_END`. Wrong-module pointers are rejected and logged without entering `GetRealAddress`. The resolved target must belong to the engine executable before hook installation. No assertion is disabled, no library is patched, and no engine RVA or offset is hardcoded. Export names and table bounds come from the actual library. Initialization logging uses RE_Kenshi's log API, not gameplay calls.

## Final binary evidence

`Verification/HookAddressVerification.txt` in the source ZIP contains the complete final `startPlugin()` disassembly, DLL hashes and checks. Each getter call is immediately preceded by **MOV RCX, [KenshiLib IAT entry]**, not LEA of a local thunk. Windows fills these entries with the actual exported KenshiLib addresses when loading the plugin; the startup guard verifies those relocated values at runtime.

| Hook | Argument MOV RVA in LootAll | GetRealAddress call RVA | IAT slot RVA in LootAll | Exported stub RVA in inspected KenshiLib |
|---|---:|---:|---:|---:|
| GameWorld lifecycle | 0x1228A | 0x12291 | 0x18120 | 0x1AF64 |
| InventoryGUI show | 0x124B0 | 0x124B7 | 0x18208 | 0x1BF36 |
| InventoryGUI destructor | 0x126E8 | 0x126EF | 0x180F8 | 0x1BFC6 |
| InventoryGUI update | 0x12878 | 0x1287F | 0x18108 | 0x1BF30 |

All four exports are generated indirect-jump stubs inside the inspected library's exported table `[0x15B46, 0x20DEA)`. These numbers are observations for this binary only; runtime code does not use them. Source audit found one centralized `GetRealAddress` expression, instantiated for all four hooks, and no additional resolver calls. The post-build scanner checks every executable PE section for references to the getter's IAT entry and fails on extra, missing or unreviewed call forms.

## Checks actually completed

- Compiled all 12 C++ implementation files and linked the complete DLL with Microsoft C/C++ 16.00.30319.01 x64, `/O2 /GL /MD /EHsc`, `/LTCG`, Unicode and the release VC2010 STL ABI. Existing compile-time ABI assertions pass. No compiler/linker errors; the existing upstream MyGUI C4091 warning remains.
- Executed the full existing configuration/filter suite: **35 checks passed**.
- Executed **8 binary regression checks**: accept the final DLL; reject a local-image LEA substitution at each of the four hooks, a wrong imported argument, a missing resolver call, and the original crashing 1.0.0 DLL. Mutated binaries were never executed.
- Inspected PE32+ AMD64 and the exact `?startPlugin@@YAXXZ` export. **173/173 direct imports** exist in installed game/Windows DLLs: KenshiLib 44, Kernel32 22, User32 3, MSVCP100 56, MSVCR100 48.
- Mandatory post-build hook checks compare imported symbols to real generated exports in the installed KenshiLib, inspect their table bounds and instructions, and save annotated disassembly. A build cannot report success if these checks fail.
- Compared all original source files against the preserved 1.0.0 baseline: only `Plugin.cpp` changed, and its four detour bodies are unchanged. Added `HookInstallation.h/.cpp`. All other original gameplay source, configuration defaults, tests, loader manifest and `.mod` contents remain unchanged (INI version comment updated only).
- Installation ZIP contains the complete DLL, `.mod`, INI, JSON, README, license, notices and this report. Source ZIP includes build/test/verification scripts and evidence. `LootAll-1.0.1-SHA256SUMS.txt` identifies both archives.

## Startup log and remaining runtime test

Expected progress includes:

```text
[LootAll] startPlugin entered
[LootAll] resolving GameWorld lifecycle hook
[LootAll] GameWorld lifecycle hook stub verified in KenshiLib.dll: <address>
[LootAll] GameWorld lifecycle hook installed
[LootAll] resolving InventoryGUI show hook
[LootAll] InventoryGUI show hook installed
[LootAll] resolving InventoryGUI destructor hook
[LootAll] InventoryGUI destructor hook installed
[LootAll] resolving InventoryGUI update hook
[LootAll] InventoryGUI update hook installed
[LootAll] initialization complete
```

Each inventory hook also logs its verified stub address; version/config messages are included. A failed check logs its reason and returns without activating LootAll. If a later hook fails, any already-installed detours retain their original forwarding behavior while the manager remains null.

**No Kenshi runtime success is claimed.** The game was not launched, injected into, restarted or modified for this repair. The user must test startup and the README's existing gameplay checklist, including transfers, filters, stack quantities, ownership, fallback and world/inventory cancellation. Static verification proves the corrected emitted argument path; it does not prove live hook execution or gameplay correctness on every mod combination. All existing features and previous scope limitations are preserved.
