# AlphaRing Project Handoff

## Project Overview

**AlphaRing** is a C++ DLL-based modding tool for Halo: The Master Chief Collection (MCC). This is kirklandsig's fork with controller binding features and Proton compatibility fixes.

**Repository**: https://github.com/kirklandsig/AlphaRing
**Upstream**: https://github.com/thejackbitt/AlphaRing (based on WinterSquire's original)

### Current Releases

| Tag | Status | Description |
|-----|--------|-------------|
| `stable-v1.3.5` | Stable | Last known stable release before experimental changes |
| `v1.4.0-experimental` | Testing | Proton compatibility fixes |
| `v1.4.1-experimental` | Testing | Bug fixes + custom profile presets |
| `v1.4.2-experimental` | Testing | wcstombs null-termination fix for profile save crash |
| `v1.4.3-experimental` | Testing | Added file logging for crash debugging |
| `v1.4.4-experimental` | Testing | Fixed wcstombs crash in Profile::Save() (still not fully resolved) |
| `v1.4.5-experimental` | Testing | Likely crash root-cause fix (ServiceTag %ls overread), settings data-loss fix, robustness pass, configurable hotkeys |
| `v1.5.0-experimental` | Testing | Spawn menus (CE/H2/H3/ODST, per-player controller menus), 4-player fixes (H4 black screen, CE/H2 classic, second-load hang), overlay redesign |
| `v1.6.0-experimental` | Testing | Per-player HUD (area presets, per-element, colour, controller page), H2 ultrawide HUD fix, XiaoDanny's Reach vertical split port, armed AI + weapon choice, profile/binding/patch/Proton fixes |
| `v1.7.0-experimental` | Testing | Side-by-side (Left/Right) split in CE, H2, H3, ODST (Halo 4 WIP: HUD not adapted), SPLIT setting in the player menu, dual-wield fix for old saved controls |
| `v1.8.0-experimental` | Testing | Discord fixes: split-screen crosshair/aim/HUD centred per view (H3, ODST, H4, Reach), ODST 3-4P full HUD, SPECIES per player, CE spawn Ally/weapon/placement fixes, P2-4 profile edits kept, CE side-by-side zoom/divider/3P, H2 divider, H4 side-by-side HUD fit, settings save data-loss fix |
| `v1.9.0-experimental` | Testing | Anniversary graphics with 3-4 players in Halo CE and Halo 2 (opt-in, alternating pairs), Halo 4 HUD with bars removed fits each view, Left/Right table rebuild on load, Reach crosshair answered (MCC setting) |
| `v1.9.1-experimental` | Testing | Reach split screen: black/invisible Spartans after many deaths fixed (Reach's own garbage collector starts sooner with 2+ local players) |
| `v2.0.0-experimental` | Testing | Hot join in every game (experimental), pattern-mode offsets that survive MCC updates, Players window, H2 players 3-4 beside a teammate at a mission start, H2A 3-4P at 60 fps + 2P black fix, Back switches CE/H2 graphics mid-mission, Reach skinning pool (bodies stay), H2 modded Save & Quit, Reach side-by-side HUD/radar |
| `v2.0.2-experimental` | Testing | Halo 2 on Legendary/Iron: no more checkpoint loop with 3-4 players or hot join (#8) |
| `v2.0.1-experimental` | Testing | Fixes for the reported v2.0 issues (#2 CE hot-join split, #3 CE joiner spawns, #4 CE Anniversary 3-4P views/guns, #5 H2A 3-4P brightness, #7 H4 side-by-side gun), CE 3P spawn menus in quarters |

### Branches

| Branch | Purpose |
|--------|---------|
| `main` | Primary branch (GitHub default since 2026-07-17) — day-to-day work happens here; releases are tagged from it |
| `experimental/proton-compat` | Historical — the v1.4.x development line, now folded into `main` |
| `fix/servicetag-buffer-overread` | Upstream PR branch (megabitt01/AlphaRing PR #6), delete after the PR resolves |
| `fix/unhook-on-module-unload` | Upstream PR branch (megabitt01/AlphaRing PR #24, based on his active `master-chief`), delete after the PR resolves |

Repo cleanup 2026-07-17: old unrelated-history `dev` and superseded `feature/controller-bind-and-default-mappings` deleted from GitHub (feature branch kept locally); inherited upstream tags pruned locally; all published release tags kept.

---

## Tech Stack

- C++17
- CMake build system
- Visual Studio 2022 Build Tools
- DirectX 11 hooking for UI overlay
- ImGui for the in-game interface
- XInput for controller handling
- nlohmann/json for settings serialization
- spdlog for logging

---

## Features Implemented (This Fork)

### 1. Controller-to-Player Binding (Splitscreen)
**File**: `src/mcc/splitscreen/Splitscreen.cpp`

- Each player slot has a "Bind" button next to the controller dropdown
- Click "Bind" → Press any button on a controller → Auto-assigns that controller
- No more guessing which is "Controller 1" vs "Controller 2"

### 2. Button-to-Action Binding (Gamepad Mapping)
**File**: `src/mcc/CGamepadMapping.cpp`

- Each action has a "Bind" button
- Click "Bind" → Press a button → Assigns that button to the action
- Select which controller to listen to via dropdown

### 3. Fixed Default Gamepad Mappings
**File**: `src/mcc/CGameManager.cpp`

- Previously all actions defaulted to "Left Trigger" due to `memset()` zeroing memory
- Now initializes with standard Xbox Halo controls:
  - Jump (A), Melee (B), Action (X), Change Weapon (Y)
  - Shoot (RT), Grenade (LT), Reload (RB), Switch Grenades (LB)
  - Crouch (LS), Zoom (RS), Flashlight (D-Up), Scoreboard (Back)

### 4. "None" Option for Unbound Actions
**File**: `src/mcc/CGamepadMapping.h`

- Added `None = -1` to the eButton enum
- Unmapped actions don't trigger on button presses

### 5. Reset to Defaults Button
**File**: `src/mcc/CGamepadMapping.cpp`

- One-click restore to standard Xbox Halo controls

### 6. Custom Mapping Profiles (Controller Layouts)
**Files**: `src/mcc/CGamepadMapping.cpp`, `src/mcc/settings/Settings.cpp`

- Save current button mappings as named presets (e.g., "Xbox 360", "Switch Pro")
- Load saved profiles from dropdown
- Delete unwanted profiles
- Stored in `custom_mappings.json`

### 7. Custom Profile Presets (Armor, Colors, Settings)
**Files**: `src/mcc/CUserProfile.cpp`, `src/mcc/settings/Settings.cpp`

- Save all profile settings as named presets (armor, colors, sensitivities, loadouts, etc.)
- Load/delete presets from the Profile section
- Stored in `custom_profiles.json` (separate from controller mappings)
- Includes: armor pieces, colors, sensitivities, FOV, subtitles, loadouts, keyboard mappings, volumes, etc.

### 8. Menu Navigation Fix
**File**: `src/render/imgui/ImGui.cpp`

- Fixed: Controller navigation in game pause menus was fighting with mouse input
- ImGui now skips input processing entirely when AlphaRing menu is hidden

---

## Proton Compatibility Fixes (v1.4.0+)

### The Problem
AlphaRing previously required **Proton GE 10-15** specifically. Other Proton versions would freeze on launch.

### Root Cause
`MessageBoxA()` was used for error handling. On Wine/Proton, this **freezes indefinitely** because there's no Windows dialog system.

### Fixes Applied

| File | Change | Why |
|------|--------|-----|
| `module_definition.cpp` | `MessageBoxA` → `OutputDebugStringA` | Prevents freeze on Proton |
| `Hook.cpp` | `MessageBoxA` → `LOG_ERROR` | Same freeze issue |
| `D3d11.cpp` | Release IDXGIDevice/Adapter COM objects | Memory leak fix |
| `Graphics.cpp` | Properly release RasterizerState | Memory leak fix |
| `CGamepadMapping.cpp` | `sprintf` → `snprintf` | Buffer overflow prevention |
| `CGameManager.cpp` | Bounds checks on get_profile/get_xuid | Prevent crashes |
| `CGameManager.cpp` | Null checks in get_controller | Prevent crashes |
| `ImGui.cpp` | Bounds check for pages[] array | Prevent crashes |
| `module_definition.cpp` | Fix GetFunc() bounds (`>=` not `>`) | Off-by-one error |

### Result
Now works with **any Proton version** (9.0, Experimental, GE, etc.)

---

## Profile Save/Load Fixes (v1.4.1)

### Issues Fixed

| Issue | Fix |
|-------|-----|
| `DialogueColorStyleSetting` saved but never loaded | Added to `Profile::Load()` |
| `SubtitleSetting` not captured from runtime | Added to `CaptureFromRuntime()` |
| `MouseAircraftControlsInverted` not captured | Added to `CaptureFromRuntime()` |
| Arrays not captured (Skins, LoadoutSlots, etc.) | Added `memcpy()` calls |
| Crash on incomplete JSON | Added `contains()` checks for all arrays |

### Arrays Now Properly Saved/Loaded
- `Skins[32]` - Weapon/vehicle skins
- `ServiceTag[4]` - Player service tag
- `LoadoutSlots[5]` - Halo 4/Reach loadouts
- `GameSpecific[256]` - Game-specific settings
- `CustomKeyboardMouseMappingV2[66]` - Keyboard bindings
- `buffer4[12]` - Additional data
- `WeaponDisplayOffset[5]` - Weapon position offsets

---

## Configuration Files

| File | Purpose |
|------|---------|
| `settings.json` | Full state for all 4 players (auto-saved) |
| `custom_mappings.json` | Named controller button layout presets |
| `custom_profiles.json` | Named profile presets (armor, colors, etc.) |
| `alpharing.log` | Debug log file (v1.4.3+) |
| `alpha_ring_menu.cfg` | Menu hotkey config (v1.4.5+, ported from MegaBit's fork) — keyboard key + controller combo, auto-created with defaults F4 / START+BACK |
| `alpha_ring_hud.cfg` | Per-player HUD settings (v1.6.0): `pN.scale/area(index)/recolor/hue/strength`, `pN.<element>.x/y/scale/hidden` |
| `alpha_ring_patches.cfg` | Saved Dev Tools patch on/off states (XiaoDanny, v1.6.0) — keys are `module.patch_name` lowercased with `_` for spaces |
| `alpha_ring_splitscreen.cfg` | Reach split-screen layout / FOV / config-editor values (XiaoDanny, v1.6.0) |

All files are stored in the game's `binaries/win64` folder alongside the DLL.

---

## Logging System (v1.4.3+)

**File**: `src/log/Log.cpp`

AlphaRing now writes logs to `alpharing.log` in the win64 folder for crash debugging.

### Features
- Logs flush immediately after every message (critical for crash debugging)
- Console window still opens for real-time viewing
- Detailed step-by-step logging in SaveProfile function
- Logs Apply Profile operations

### Log Locations
- **Windows**: `<MCC Install>/mcc/binaries/win64/alpharing.log`
- **Linux/Proton**: Same path within the Wine prefix

### Using Logs for Debugging
When a user reports a crash:
1. Have them install v1.4.3+
2. Reproduce the crash
3. Get the `alpharing.log` file
4. The last log entry shows where it crashed

---

## Build Instructions

### Prerequisites
- Visual Studio 2022 Build Tools
- CMake (bundled with VS: `C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe`)

### First Time Setup
```bash
cd AlphaRing
mkdir build && cd build
"<path-to-cmake>" .. -G "Visual Studio 17 2022" -A x64
```

### Build
```bash
cd build
"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin/MSBuild.exe" WTSAPI32.vcxproj -p:Configuration=Release -p:Platform=x64
```

### Output
`build/Release/WTSAPI32.dll` (and `offset_test.exe`: run it after touching an offset header, see docs/REVERSE_ENGINEERING.md "Offsets across MCC updates")

### Install
Copy to: `<MCC Install>/mcc/binaries/win64/`

---

## Linux/Steam Deck/Batocera Setup

Works with **any Proton version** (9.0, Experimental, GE, etc.)

1. Place `WTSAPI32.dll` in MCC's `binaries/win64` folder
2. Set Steam launch options:
   ```
   WINEDLLOVERRIDES="WTSAPI32=n,b" %command%
   ```
3. For non-Xbox controllers (8BitDo, etc.): Enable Steam Input
   - Steam → Settings → Controller → Enable "Xbox Configuration Support"

---

## Key Files Reference

| File | Purpose |
|------|---------|
| `src/mcc/splitscreen/Splitscreen.cpp` | Splitscreen UI, controller-to-player binding |
| `src/mcc/CGamepadMapping.cpp` | Button-to-action mapping, bind feature, custom mapping profiles |
| `src/mcc/CGamepadMapping.h` | Button enum (includes None = -1) |
| `src/mcc/CUserProfile.cpp` | Profile UI, custom profile presets |
| `src/mcc/CUserProfile.h` | Profile struct definition (note: sensitivity fields are bools!) |
| `src/mcc/CGameManager.cpp` | Player profiles, default mappings, XUID management |
| `src/mcc/settings/Settings.cpp` | JSON save/load, custom mapping & profile presets |
| `src/mcc/settings/Settings.h` | Settings namespace declarations |
| `src/log/Log.cpp` | Logging system, file + console sinks |
| `src/input/MenuConfig.cpp` | Menu hotkey config file parsing (ported from MegaBit's fork) |
| `src/render/imgui/ImGui.cpp` | ImGui rendering, input isolation fix |
| `src/wrapper/module_definition.cpp` | DLL proxy, Proton-compatible error handling |
| `src/hook/Hook.cpp` | Function hooking, version detection |
| `src/input/Input.cpp` | XInput wrapper, menu toggle |
| `src/mcc/spawn/Spawn.cpp` / `PlayerMenu.cpp` | Spawn catalog + F4 Spawn window / per-player controller menus |
| `src/mcc/spawn/Command.cpp` | Fixed game-thread command handlers (scheduler or console hook) |
| `src/mcc/spawn/halo1.cpp`, `halo2.cpp`, `gen3.cpp` | Per-engine spawn backends (gen3 = Halo 3 + ODST) |
| `src/mcc/module/entry/entry.cpp`, `CModule.cpp` | Hook install/removal per game DLL (removed on unload); `EntryFeature` groups a file's hooks |
| `lib/game/inc/Offset.h`, `lib/game/inc/<version>/offset_*.h` | `DefOffset` offsets (objects that read as their RVA), `Feature`, `Found` |
| `src/offsets/` | Offset lookup on an unknown MCC build: known-build check, pattern scan (`Pattern.cpp`), per-module tables |
| `lib/game/inc/<version>/patterns_*.inc` | Generated byte patterns per module (`tools/offsets/gen_patterns.py`, don't edit) |
| `tools/offsets/` | `gen_patterns.py` (tables), `offset_test.cpp` (the DLL's lookup over the installed game), `perturb.py` (fake update), `check_gates.py` (every offset use gated) |
| `src/mcc/CGameManagerSplitscreen.cpp` | Player count (H4 deferred join), per-player input (spawn menu interception) |

---

## Git Workflow

### Creating a Stable Snapshot
```bash
git tag -a stable-v1.x.x -m "Description"
git push origin stable-v1.x.x
```

### Creating Experimental Release
```bash
git tag -a v1.x.x-experimental -m "Description"
git push origin v1.x.x-experimental
# Then create release on GitHub with the DLL
```

### Rollback to Stable
```bash
git checkout stable-v1.3.5
```

---

## Known Limitations

1. **4 Players Maximum** - Game engine limitation (hardcoded view bounds)
1b. **CE/H2A with 3-4 players run Classic graphics by default** (the Anniversary renderers only draw 2 views); both can keep Anniversary as an opt-in (v1.9, "ANNIV 3-4P": alternating pairs, each view at half the frame rate; H2A ~30 fps on the box). CE/H2 need the Good Luck Cairo Workshop co-op fix mods for proper P3/P4 spawns, but mod campaigns always run Classic: in Anniversary, H2's P3/P4 join at the first co-op respawn (their cells stay black until then)
1c. **H2 co-op mod: Save & Quit hangs with 2+ players** (see 2026-09-24)
1d. **Spawned characters borrow an empty mission squad** (H2/H3/ODST) - a mission script could wait on it while a spawned ally lives; allies fight but don't follow the player
1e. **Spawn menu 3-player layout is assumed to be quarters** (2 players = stacked halves, verified via the black-bar patches) - unverified in game
2. **4 Controllers Maximum** - XInput limitation
3. **Controllers must be connected before launch** - Still investigating
4. **Profile UI shows raw indices** - Armor/skin selection shows index numbers, not filtered by slot

---

## Known Issues Under Investigation

### Profile Save Crash (Reported - Windows) — LIKELY ROOT CAUSE FIXED (2026-07-17)
- User reported crash when: Enable "Use player1's profile" → Apply Profile → Save Profile/Preset
- Unable to reproduce locally
- **v1.4.2:** Fixed `wcstombs` null-termination in `CustomProfile::SaveProfile()` (preset save path)
- **v1.4.3:** Added file logging to `alpharing.log` for crash debugging
- **v1.4.4:** Log from user showed crash was in `Profile::Save()` (the "Save Profile" button), not the preset path. Fixed `wcstombs` there too + added null check in `CaptureFromRuntime()`
- **2026-07-17 (v1.4.5 pending):** Deep review found the last unguarded instance of this bug class: `CUserProfile.cpp:94` did `sprintf(buffer, "%ls", ServiceTag)` on the non-null-terminated `wchar_t ServiceTag[4]`. "Apply Profile" memcpys the LIVE engine profile (with a real, full 4-char service tag) into our container, and the Profile UI then renders that tag every frame — `%ls` reads past the array and can smash the stack. This explains why it never reproduced locally (our zero-initialized test profiles have empty tags) and why the v1.4.2/v1.4.4 fixes (save paths only) didn't stop it. Also fixed: the ServiceTag edit box left stale trailing chars when shortening a tag (could re-trigger the same bug), and loads truncated legit 4-char tags to 3 (`mbstowcs_s` terminator-slot off-by-one, 2 sites).
- **Verify with user once v1.4.5 ships**

### Sensitivity Options Don't Work (Reported) - resolved in v1.5.0
Look sensitivity bytes (0x1B5/0x1B6) were bools and players 2-4 got zeroed container profiles; both fixed in v1.5.0 (sliders 1-10, profiles seeded from player 1). Re-check per player on the box if a new report comes in.

---

## Session History

### 2026-10-01 - upstream PR prep for MegaBit; v2.0.2 (Halo 2 Legendary/Iron checkpoint loop, #8)

The user sent MegaBit the join-forces reply and is in his dev channel; MegaBit answered the open questions there -
his answers are in `docs/UPSTREAM_SYNC.md` (local, out of git). PRs go to `master-chief`. Local prep (user's go): vcpkg at `C:\Users\yanal\dev\vcpkg`; his tree in a worktree
`C:\Users\yanal\dev\alpharing-mc` builds clean with `-DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake
-DVCPKG_TARGET_TRIPLET=x64-windows-static-md`; PR branches (worktrees `alpharing-pr`, `-pr02`, `-pr03`):
`upstream/00b-servicetag` (pushed as `fix/servicetag-master-chief`, opened as megabitt01/AlphaRing#26; #6 closed with
a pointer - a force-push re-point was blocked by the auto-mode classifier), `upstream/01-halo2-fixes` (Save & Quit
patch + players 3-4 hook - his first Halo 2 EntrySet, needs #24 first; Codex: approve after the gate below),
`upstream/02-reach-fixes` (skinning pool, side-by-side HUD centring, CPatch capture, g_writes lock; 2 small
conflicts with #24), `upstream/03-input-startup` (8 commits: XInput slots, hotkeys, unsupported build runs unmodded,
logging, WndProc, MenuConfig, mutex define, player count clamp). All build; box runs on his build pending.
**v2.0.2:** Codex's review of the Halo 2 port found that clearing flag 8 on Legendary/Iron (no co-op respawn there)
leaves a player who never spawns and counts as dead, and any dead player reverts the checkpoint there. Box-confirmed
on v2.0.1 (memory reads: Legendary 3P, P3 never spawned, revert every 30-45 s; hot join Legendary at 1P, the same)
and fixed with the game's own gate (`0x6A6320` && `0x6A5D40`); box: Legendary 3P fine, hot join Legendary fine (a
joiner comes in at a starting place), Normal 4P unchanged. Codex approve. Harness: `h2_state.py` (via mem.py),
`h2_fresh_diff.sh` (env DIFF), `h2_legend.sh <dll> <players> <hotjoin> <tag>`. Side finding (not fixed): `assertm` is
live in Release builds in both trees, though our `main.cpp` comment says otherwise. Box: X server hit 255 clients
after many restarts (Steam leak) - cleared by killing Steam/Wine by PID.

### 2026-09-30 - v2.0.1: the open reports fixed (#4, #5, #7), GitHub issues - RELEASED as v2.0.1-experimental (commit ccd9127)

GitHub: Issues turned on for kirklandsig/AlphaRing (the user asked); #2-#7 filed from the user's reports with
symptoms and screenshots (`doc/images/issues/`, commit e8f9ae7), and the v2.0.0 release page has a "Reported issues"
list. On the release: comment each fixed issue's cause and fix (drafts `scratchpad/issues/fix_4.md`, `fix_5.md`,
`fix_7.md`; #2/#3 from the night entry), close as completed, update the release page lines.
Fixed (box-verified; see REVERSE_ENGINEERING.md for the mechanisms):
- **#4 Halo CE Anniversary 3-4P** - the night entry's "phase between threads" theory was wrong (a TEMP 0.3 ms stall
  before the FP hand-off changed nothing). Two real causes: (a) the game's hand-over queues the worker's list build
  (`0x3BBA10`) right after views 0-1, and our hook handed views 2-3 over after it returned (2351/4200 stale) -> views
  2-3 now go over from a hook on the per-view hand-over `0xB29268`, right after view 1 (2 of 7200 early, both at
  P3's join; /simplify's altitude review replaced a first version where the build waited for the sync); (b) first-person models are per (slot, object) with two slots, while the ammo display,
  weapon events and camo go by local player index -> pair 2's syncs hand over as slots 2-3 (`entry_first_person_sync`
  on `0x7AC60`, the `cmp r15d,1` at `0x7AE64` widened to 3 while quad runs, slot 2 shown in view 0),
  `FirstPersonPair` now copies only the objects. A first build crashed (stale view-3 object handed over with -1 ->
  null memcpy); a missing view's object is -1 in both places now. Box: P3/P1 rifles count their own rounds, 0 of
  1200 frames without P1's gun (P3 unspawned), 2-min soak, leave/rejoin, Back cycle.
- **#5 Halo 2 Anniversary 3-4P brightness** - the AHDR reads each view's luminance back a frame late, i.e. the other
  pair's -> `entry_hdr_pass` (`0x210BF0`) swaps in pair 2's adaptation state and views 2-3's read-backs on pair 2's
  frames. Box: P3 62.4-62.8 regardless of P1 (before 70.6/53.9/65.1/54.6), P1's sky 87 (before 98), 90-s soak. The
  plasma-light "bleed" is normal lighting; "missing effects" not tested on their own. Left open (minor): four effect
  readers of +0x44 (`0x14BD20`, `0x16FBA0`, `0x3CE210`, `0x4FD540`) run while the next list is built (TEMP probe
  `temp_h2_expo.py`: none on the render thread inside the frame), so a whole-frame swap was rejected; they'd need the
  list's pair in each reader.
- **#7 Halo 4 side by side gun/arms** - `0x34EC44` scales the viewmodel FOV by screen/view aspect (x2 in a 960x1080
  half) -> `ViewmodelAspect` switches the divss's modrm `E8 -> E9` (`0x34ED99`) while Left/Right is on screen,
  restored otherwise and on unload. Box: 3P side by side, E9 live on the fix, E8 on v2.0. The 4P path (patch off) is
  untested.
Not fixed: **#6 Halo 2 Anniversary 1P black** - nine tries without a repro (incl. the old logging build: not
deterministic either). Ruled out: Present/ImGui on another thread (same thread), the cinematic check (a plain global).
Reviews: Codex pass 5 approve; a Claude reviewer (WaitForSync could overlap a late build with its resubmission ->
moot after the rework; the H2 effect readers -> probed, documented); /simplify (4 agents: CPatch::apply flushes the
instruction cache itself, views 2-3 handed over inside the game's hand-over instead of WaitForSync, H4 switch-off =
restore, LeftRight::Players public, swap_ranges exchange, the unused node offsets dropped); Codex pass 6 approve, no
findings. Release candidate: `scratchpad/WTSAPI32_fixE.dll` (md5 fa093a66...), box-tested: H2A 3P brightness (P3
62.4-62.8, P1 sky 87) + soak; CE 3P regression (`ce_regress_v201.sh`). New harness
(scratchpad and box, never in git): `quadcfg.sh`, `ce_quad3.sh`, `h2_quad3.sh`, `h2_1p_anniv.sh`, `gunframes.py`,
`cellluma.py`, `hdrwatch.py`/`hdr4.py`/`tbl4.py`/`camread.py`, `tbuild.sh`, `da.py`/`xrange.py`/`dispfind.py`/`ptrfind.py`,
`h2_expo_test.sh` (P3's brightness while P1 turns), `ce_regress_v201.sh <dll> <tag>` (CE 3P: guns, ammo display,
leave/rejoin, Back, soak), TEMP `temp_ce_camrace2.py` (stale camera copies), `temp_h2_expo.py` (the effect readers).

### 2026-09-29/30 (night) - v2.0.1 candidate: fixes from the user's live testing - released in v2.0.1-experimental

The user tested v2.0 on the box (hot join on, Anniversary 3-4P on, side by side) and handed the box over for the night.
Found and fixed (box-verified with the harness; release candidate `scratchpad/WTSAPI32_v201_rc3.dll`, md5 4e91b08c...,
a clean build without TEMP code; Codex pass 4 approved it):
- **Halo CE Anniversary + hot join, one player split in two** (frozen second view, full-screen HUD across both; a
  leave left the split). Halo CE sets its Saber split flag (`0x2E3B821`) only while the map loads (`0x66DF0`:
  `setg` on local count > 1 at `0x6762A`), and hot join loads with four slots. `halo1/hotjoin.cpp` `FollowSplit`
  (each tick, after holding the count): sets the flag from the held count and applies it with the game's own
  `set_split` (`0x4150F0`: adds/drops camera 1, lays the views out; anniversary.cpp's detour makes/drops cameras 3-4).
  First written in anniversary.cpp's sync; /simplify moved it so it doesn't depend on the quad feature's offsets. Box:
  fresh Halo, 1P full screen, 1->2->3 split/quad, 3->2->1 back (Players window B), Back to Classic at 1P, a join in
  Classic, Back to Anniversary at 2P (stacked).
- **Halo CE joiners waited on occupied starting places** (Halo has two, both in the lifepod, where P1/P2 stood
  within 0.05 units; the joiner had no unit, grey view, until a checkpoint revert or someone moved). `halo1/hotjoin.cpp`:
  trail spots kept a stride (0.8) apart so standing still keeps them, only spots within 10 units of the teammate
  (Codex: old spots could outlive a teleport/checkpoint), and a joiner's first spawn is put on a trail spot through
  `choose_start` (`0xAD39AC` -> index 0) and `start_location` (`0xAD3940` -> a copy of entry 0 with our point and a
  facing toward the teammate), only during that `player_spawn` call. New DefOffsets + patterns (92/92). Box: P2/P3
  spawned 0.8/1.9 units behind P1 on Halo's opening path, facing him.
- **Spawn menus with 3 players in CE Anniversary drawn in the wrong corners** (laid out as the Classic 3P layout;
  CE's quad mode shows quarters; H2A keeps H2's own 3P grid, player 1 on top - Codex pass 2 caught an H2 regression
  in the first version). `Splitscreen::AnniversaryQuartersShown` (CE's `Anniversary::QuadShown`, the module's own
  `s_sync_quad`) + `PlayerMenu` layout. Box: each menu in its quarter; H2A 3P menus still P1 top / P2-P3 bottom.
  Review: Codex passes 1-3 (1: trail spots outliving teleports -> kNear; 2: the H2 regression; 3: approve), /simplify
  (4 agents: split fix moved to hotjoin.cpp, spawn spot as one struct, quarters from the module's own flag).
Not fixed that night (#4, #5, #7 fixed on 2026-09-30, see above; the notes below are superseded):
- **3-4P Anniversary first-person mix-ups** (P1's gun gone while P3 was in a Banshee; P2's plasma rifle through P3's
  gun in H2; flicker, H2 brightness/pulsing). TEMP trace (CE): sync -> hand_over run every frame on the game thread
  (hand_over is inside the sync), the camera list is built on worker threads meanwhile, and the render thread draws
  the previous list while the next sync overwrites the per-slot first-person state (object ids per slot are shared
  per weapon type - P1 and P2's ARs were the same object - with per-slot nodes; FirstPersonPair writes pair 2's into
  slots 0/1). Visible or not depending on the phase between the threads (a 1-s capture with P3 on the pistol showed no
  swap). Options: serialize the sync with the render thread in quad mode (fps cost unknown) or four slots (engine).
- **Halo 2 Anniversary black at one player** (the user's session, after CE/H4/Reach): not reproduced in six fresh
  tries with the release DLL (hot join on/off, launched Remastered or Back, after a Reach hot-join game). A TEMP build
  with LOG_INFO in the H2 Anniversary render hooks (hand_over, build_list, composite, frame) was black by itself -
  hot join on or off - while variants without that logging rendered: likely timing-dependent, a race in those hooks.
  Don't put logging in those hooks for diagnostics.
- **Halo 4 side by side, full-height view: first-person gun and arms magnified** (the v1.9 README picture shows it
  too). Our code sizes the views/HUD, not H4's viewmodel projection; not investigated.
Also: README v2.0 sections got the screenshots the user asked for (Players window, Splitscreen > Options, hot join
in game, HUD window, spawn menus: `doc/images/players-window.jpg`, `splitscreen-options.jpg`, `hot-join-players.jpg`,
`overlay-hud-window.jpg`, `spawn-menus-anniversary-3p.jpg`, cropped to keep the gamertag out). MegaBit answered the
join-forces thread; his answer and a reply draft are in `docs/UPSTREAM_SYNC.md` (local, kept out of git), not sent. Harness: `ce_fresh.sh`/`h2_fresh.sh`/`reach_fresh.sh [dll]`
(PLAYERS=n), `temp_ce_fp.py`, `temp_h2_black.py`, `temp_h2_off.py`, `temp_h2_bisect.py`, `temp_h2_leafvar.py`, `xref.py`.

### 2026-09-29 (late) - Upstream sync plan with megabitt01 - planning only, nothing sent

User: plan how to send our work to MegaBit as PRs, but don't send anything yet (target branch unknown); draft a
message proposing the plan. Written up in `docs/UPSTREAM_SYNC.md` (local, kept out of git - it holds his Discord
answer and our drafts): fork inventory (his April port = our 2026-02-01
state; his dashboard replaced our Settings window/JSON; both carry XiaoDanny's Reach work), overlap table, the
questions for him, a 10-PR series (fixes first, then offsets, then features), how each PR is built/tested, and the
draft Discord message. Findings: his default branch is now `master-chief`; a straight merge conflicts in 41 files;
he squash-merges (so no stacked PRs); the ServiceTag bug from #6 is still in `master-chief`; building his tree needs
vcpkg (SDL2, SDL2_mixer), not installed here.

### 2026-09-28 - v2.0 item 2: offsets found again after an MCC update (pattern mode) - uncommitted, awaiting box test

User's approved v2.0 plan, item 2: survive MCC updates by resolving offsets from byte patterns at runtime.

- **Offsets are objects:** `DefOffset(OFFSET_X, 0x…)` in the headers (lib/game/inc/Offset.h); code keeps `module + OFFSET_X`. The inline literals moved into the headers (Reach black bars/CUI/HUD/RT sizing/Dev Tools dump, Halo 3 log/TAS hooks and natives, MCC data pointers and `get_index_by_xuid`, the black-bar table patches, H2's six "== 2" test sites).
- **Known build = no change:** a module (or MCC's exe) whose PE timestamp/size is 1.3528's keeps the written values without scanning. Another build is scanned once as it loads (src/offsets, 4-23 ms per module in offset_test) and each offset found or marked missing, all logged `[Offsets]`. WS 1.3498 keeps its own exe offsets. An unknown exe is pattern mode instead of "disabled", which now only happens when an exe offset outside a small optional set isn't found.
- **Missing offsets switch their feature off:** `EntryFeature(name, offsets)` per hook file/namespace (a missing one skips all its hooks), embed patches not captured, patch.xml off on unknown builds, MCC exe hooks skipped, spawn backends/HUD/config store/natives gated. `tools/offsets/check_gates.py` enforces it. The ODST render hook moved into halo3odst/splitscreen.cpp (as Halo 3's), render.cpp removed.
- **/simplify pass done** (4 reviewers): shared PE/build helpers, `found()` derived from the value, lint reads every gate form, generator uses a sorted anchor index.
- **Tables:** `tools/offsets/gen_patterns.py` -> `lib/game/inc/1.3528.0.0/patterns_*.inc`; 351 of 362 offsets patterned (310 with two independent patterns). Not patterned: 10 Windows Store exe offsets, `OFFSET_HALO2_PF_AI_LIVING_COUNT` (13-byte thunk; H2 character spawning off on another build).
- **Verified:** offset_test.exe: every pattern of all 351 resolves exactly on 1.3528. perturb.py (code shifted + data moved + patterns broken/duplicated, all 8 modules, two seeds): every offset at its new address or reported missing where its only pattern was broken, none wrong; with half of all functions' bytes scrambled on top (`--churn 0.5`) 336/351 still found, 15 missing, none wrong. Release build compiles; header values unchanged (script-checked).
- **Box-test next:** a normal session on 1.3528 first (identical behaviour expected, no `[Offsets]` lines in the log), then a build with `DebugFlags::g_forceOffsetLookup = true`, which looks every offset up from its patterns on the known build (log: each "at X (was X)") so every feature runs off looked-up offsets.

### 2026-09-28/29 - v2.0: open reports, pattern-mode offsets, regression check, CI, installer, join screen, hot join - RELEASED as v2.0.0-experimental (commit 63ca81a)

User: "These all sound awesome. Go for it." / "I think this would be a worthy v2.0". Plan in memory `v20-plan.md`.

- **Reach HUD (salty's 21:9 + the 2P bottom-view squeeze):** `hud_anchor.cpp` centres the Left/Right HUD box too (`CentreHudBox`); `Hud.cpp` skips Reach's per-widget motion-sensor transform (radar got per-player offsets twice) and puts Reach's grenades on the left side. The "squeeze" was player 2's saved "HUD area: 4:3 box" preset on the box hitting both bugs. Box-verified 1080p and a 1920x822 21:9 window. Details: docs/REVERSE_ENGINEERING.md.
- **Halo 2 Save & Quit hang (SR388):** modded campaign + 2+ players; Halo 2 treats the game as from its own lobby and loads its menu map. Embed patch "Save & Quit to MCC" (`OFFSET_HALO2_PF_QUIT_TO_LOBBY_TEST` 0x679EE0). Box-verified 2P and 4P with the co-op fixes mod. Root cause traced with the new Wine stack unwinder and temporary hooks (docs).
- **Pattern-mode offsets (agent, merged):** `DefOffset` objects, `src/offsets/*` scanner, `patterns_*.inc` generated by `tools/offsets/gen_patterns.py`, `offset_test` (355/366 offsets patterned; exact on 1.3528), `check_gates.py` lint. Forced-pattern box regression run: see results line below. New offsets need a `gen_patterns.py` run (~10-15 min) and, for patched sites, a COVER entry.
- **Regression check:** harness `regress.sh` + `regress_check.py` (6 games x 2/4 players, ~45 min). Merged DLL: no regressions (known: built-in CE/H2 P3-4 before a co-op respawn; H3 intro cinematic).
- **CI:** `.github/workflows/build.yml` (Windows runner: gate lint, Release build, DLL + SHA-256 artifact). Dry-run locally from a clean configure; not yet run on GitHub (needs a push).
- **Installer:** `tools/install/install_linux.py` (Steam Deck/Batocera/Flatpak/Snap; MCC lookup via libraryfolders.vdf, DLL from the latest release or a file, backup, launch options only while Steam is closed, `--skip-eac-prompt`, `--dry-run`, `--uninstall`). Dry-run + VDF round-trip verified on the box.
- **Join screen:** `src/mcc/splitscreen/Join.cpp` - Players window (A join, B leave with compaction, Start saves and closes; opens with the overlay at MCC's menus). Box-verified with the virtual pads (note: vpad N is not XInput slot N under Wine). Players-window lineup also checked in a real Reach mission (each pad moves only its view).
- **Species (user asked to test it for real):** works in H2 Classic, H3, Reach, but only from the next game (the games read profiles as they load; a respawn keeps the old species) - hint now "Species changes next game", README note, new reach-species.jpg. Reach's Team Slayer variants (TU Team Slayer DMR) force Spartans; v1.8's "verified" was a misread viewmodel there. Checked via biped tags (harness `bipeds.py`) and H2's purple Elite HUD. Reach's next-game switch: confirm in the final run.
- **Item 5a - Reach's real drawing limit FOUND and raised:** the per-frame skinning pool (0x35C00 bytes at 0xC51C40, allocator 0x25093C). `haloreach/skinning.cpp` moves it to a 4 MB buffer at module load (EntrySet::on_add, new) - 10 lea + 10 mask sites + limit + handle shift, strict all-or-nothing, restored on unload. v1.9.1's collector limits REMOVED (world.cpp back to v1.9.0; offsets PF_COLLECT_GARBAGE/V_GARBAGE_COLLECT_* /TLS_PLAYERS gone). Box: 4P at the stock collector's full pile (118 garbage, ~510 objects) every view fine, pool high-water 329 K; ~45 fps (GPU-bound) with the pile. Patterns regenerated (Reach 83/83), offset_test OK. Docs updated. Build: scratchpad WTSAPI32_v20_d.dll.
- **Item 5b - Halo 2 Anniversary 3-4P at 60 fps (done):** views were drawn full width x half height and squeezed into cells (30 fps @ 82% GPU). `halo2/anniversary.cpp` records every `_split` render target as `0xF7750` adds it and, while quad mode lasts, remakes them at half width (vtable +0xC8 release, clear bit 27 of +0x98, +0xA0 with the new size). Box: 60 fps (vsync) @ 69% GPU at the same spot, all four cells whole, Classic and back keeps them narrowed. Docs: "Halo 2 Anniversary graphics in split screen (v2.0)".
- **H2 Anniversary at 2 players was black (fixed):** the composite quad leaves no pixels (megabitt01 found the same in #23, credited). The quad is collapsed and each view drawn into its half by our shaders (`CollapsedImage` + `DrawCell`). Box: both halves drawn (brightness 84/95), 30 fps @ 82% GPU - H2A 2P's own cost, stock.
- **Item 5c - CE Anniversary all four views every frame: NOT done, findings documented.** Four cameras in one list crash the commit (`0x455170` -> `0x2E2480` -> `0x2B0090`): the light manager (`0x1BEA9D0`) holds per-view light lists for exactly two views, and hidden flags / first-person models are two-slot too. Rendering twice would need all of those swapped mid-frame (days of RE). Presenting only complete frames was measured: presents halve, the loop stays at 60 (MCC caps it; `GameUserSettings.ini` vsync isn't what it reads), so no gain. CE stays on v1.9's alternating pairs; README Known limitations says so. 2026-09-29 morning (user: "I think you could fix the Halo CE renderer before v2.0"): running the game frame `0x87F90` twice per MCC loop halves the loop (each frame waits ~16.7 ms for MCC's pacing, which isn't Present and ignores Engine.ini t.MaxFPS / rhi.SyncInterval / r.VSync / bSmoothFrameRate - likely UE4's RHI vsync pacer in the exe); box restored after (Engine.ini backup `alpharing-test/Engine.ini.bak0929`). Docs: "Halo CE Anniversary: why the four views aren't all drawn every frame". TEMP experiments (never shipped): scratchpad `temp_ce4.py`, `temp_ce_twice.py`, `temp_ce_present.py`, `TEMP_crashlog.cpp`.
- **Codex adversarial review, two passes (all fixed, box-verified):**
  1. H2 texture lifetime: a held reference doesn't stop the engine deleting a texture (`0xF8750` -> `0x1A5AC0` -> `0x1A5990` deletes whatever `+0x78` says), and the unload dropped references into already-freed textures. Now a hook on `0x1A5990` (`OFFSET_HALO2_PF_SABER_DELETE_TEXTURE`, patterned) forgets the texture under the lists' mutex first. Box (TEMP log build): Save & Quit deleted all 48 tracked textures through the hook on thread 1440 (narrowing runs on the render thread), same-process reload narrowed 48 new ones, no access violations.
  2. Halo 3 with `OFFSET_HALO3_PF_ENGINE` missing (pattern mode): `teb_data()` dereferenced a null TLS pointer. Now it and the globals return null until the engine hook ran; bump possession depends on the engine offset. Box with the offset forced missing: unguarded build crashed loading a mission (negative control), guarded build played and opened the overlay (whose curve backend reads the globals every frame).
  3. Players window: in a game it only shows the lineup (join/leave at MCC's menus; the game reads controllers live). A fills the first player without a controller, else appends. Buttons held when the window starts reading pads aren't presses (the overlay's Start+Back chord closed it again). Box: in a mission B on pads 2-4 changed nothing; at the menus A on the free pad filled P4 ("Controller 3", saved). The Start+Back chord itself couldn't be produced on the box (no virtual pad is XInput slot 0 there) - not box-verified.
  4. Installer: uninstall only replaces/removes a DLL identified as AlphaRing (repeat uninstall and other mods safe), an older AlphaRing isn't backed up as "the original", backup and install are written whole (copy to `.partial`, then replace), the launch option merges into an existing `WINEDLLOVERRIDES` (and uninstall removes only our entry), Steam's config written whole. Sandbox-tested (8 install/uninstall sequences, a failed copy and retry, 5 launch-option cases).
  Second pass found nothing further in the H2 locking or the Halo 3 consumers.
- **Simplify pass (4 reviewers):** skinning uses `CPatch::apply` and `Offsets::NtHeaders`; H2's composite paths share `CollapsedImage`; H2's feature list no longer repeats its hooks' own targets; Reach HUD bounds read through `LeftRight::Rect`; CModule's black-bar entries from `SplitscreenConfigStore` constants; small cleanups (Hook.cpp, Pattern.cpp, Module.cpp, installer `load_vdf`). Skipped: `Entry::m_offset` -> module base (wide), MCC exe `Optional()` list redesign, a `LeftRight` Feature derived from `Gen3`, caching skinning's 8-13 ms scan per Reach load, gen_patterns/offset_test tidy-ups.
- **Verification (box, RC3 = the release candidate):** offset_test all modules exact; perturb (H2, Reach) 0 wrong; check_gates clean; H2A 2P and 4P as above; Reach pool log "0x35C00 -> 0x400000 bytes (10 users, 10 offsets)"; full regression 2P/4P x 6 games: Reach, CE, H2, H4 ok at both; ODST 4P ok; the checker's "no HUD?" flags were H3's intro cinematic (2P both shots, 4P first shot - the second 4P shot has all four HUDs) and a red damage tint on ODST 2P view 1 (HUD visible); H3 2P rerun past the intro: both HUDs, 60 fps. Build = RC3 (md5 f34db68e...), no source changed after it.
- **Not done tonight:** Reach's SPECIES switch at the next game (MCC's custom-game lobby didn't take the virtual pad's variant change reliably; the species code is unchanged since the earlier check - Reach P2 Elite set before a game, H2/H3 next-game switch); the overlay's Start+Back chord with the Players window (not producible on the box).
- **Box state:** published v1.9.1 DLL installed (release asset md5 108033f9...), cfg-backup-0926 restored, alpha_ring_patches.cfg and flag files removed, GameUserSettings.ini restored, vpad.py stopped, MCC quit to desktop. RC3 kept at `/userdata/system/alpharing-test/WTSAPI32_v20_rc3.dll`.
- **Commit notes:** add the untracked `src/mcc/module/entry/haloreach/skinning.cpp`; credit megabitt01/AlphaRing#23 for the H2A 2P black screen.
- **2026-09-29 (user testing on the box):** dual wielding confirmed by the user in Halo 3 and Halo 2 (P3 dual plasma pistols). The user found Back (Classic -> Anniversary mid-mission) black in Halo 2 at 3 players: with 3-4 players the quad mode only ran if chosen before the mission, never with side by side chosen (the box's layout), and CE never followed a switch from a Classic start. Fixed (user: "fix the back button stuff ... on both Halo 2 and Halo CE"): the 3-4 player mode is set up for every 3-4 player mission and runs while Anniversary is on screen (CE's flag `OFFSET_HALO1_PV_ANNIVERSARY_SHOWN` 0x1B7AA84, found by diffing halo1.dll's data across Back presses; CE's extra cameras made at split start via `QuadReady`); CE/H2 side by side is on screen only while Classic is (`LeftRight::OnScreen` asks `Anniversary::ClassicShown()`, an atomic the split-screen hooks note - Codex: reading the module from the present thread raced its unload); the saved choice only decides whether missions start in Anniversary; tooltips/README updated. Box (harness `backtest.sh` in the scratchpad: Classic -> Back -> Anniversary -> Back -> Classic, per-view brightness + screenshots): CE and H2 at 2 (side by side), 3 (side by side and stacked, option off) and 4 players (option on and off) all drew every view in every state; CE 2P Anniversary HUD no longer stretched. Build RC6 = scratchpad `WTSAPI32_v20_rc6.dll`.
- **2026-09-29 (later) - Hot join in every game** (user: "what are the odds that we could have live in-game add and drop of players", then CE prototype, then "can you go ahead and just add that to all of the other games on MCC"). Option Splitscreen > Options > Hot join (config `-1.hot_join`). `Join.cpp` `HotJoinPoll` (every Present): in a running map (`CGameManager::running()`), A on a controller no player uses raises the player count (saved); B in the Players window lowers it - Halo CE only (`HotJoinLeaves`), since MCC ends a Gen3 mission when a local user signs out (box: H3 "Leaving..." -> Exiting). H3/ODST/Reach/H4 take the new MCC user natively. CE/H2 make players at load: MCC is told of four (`HotJoinSlots`), `halo1/hotjoin.cpp` / `halo2/hotjoin.cpp` hold the engine count at the joined count and block unjoined spawns. Placement: H2 clears player flag 8 (+6) for players without a starting place or joining late so the campaign co-op respawn (0x6A1320) puts them beside a teammate - also fixes megabitt01's report (players 3-4 in the wrong spot at a built-in mission start; Discord reply posted 2026-09-29 13:41 with the user's OK); CE has no such respawn, so the joiner's new unit is moved onto a teammate's recent spot (`object_set_position` 0xB359E8). Details: docs "Hot join: local players in the middle of a mission (v2.0)". Box (build `WTSAPI32_hj11.dll`): join + turn (per-view screenshot diff vs a no-input baseline) + fire (ammo counter) in all six games; H2/H3 soaks (3 and 2 min); CE leave/rejoin with HUD relayout; H3 fresh 1P start -> option turned on in-game -> join (1 -> 2 views); negative control: option off, A on every pad joined nobody; B does nothing outside CE (H2, H4); H2 fresh 4P Delta Halo start: P3/P4 1.8-2.4 units from P1 at once. Harness: scratchpad `joinpads.sh`, `movecheck.sh`/`qdiff.py`, `firecheck.sh`, `soak.sh`, `h2_pos.py`, `ce_units.py`. Gotchas: the virtual pads' XInput slots reshuffle per MCC start (joinpads prints the mapping); H3's Needler ammo readout doesn't show harness fire - test with the plasma rifle; resuming a checkpoint with fewer players than it was saved with exits (H3) or stays black (Reach) - pre-existing; the user OK'd starting missions from MISSIONS on the box ("Current saves are all for testing"). Commit notes: add untracked `halo1/hotjoin.cpp` and `halo2/hotjoin.cpp`.
- **Hot join review round (Codex pass 1 + /simplify, 4 agents), build `WTSAPI32_hj14.dll`:** HotJoinPoll waits for `MCC::Ready()` (Present is hooked before MCC's init finishes); joins/leaves reuse the Players window's `Join`/`Leave` (controller before count, release fence) and the settings save waits until the map stops running (it cost ~30 ms mid-mission); the option is locked in a CE/H2 game (their modules latch it at mission start; turning it off would drop MCC's reserved users and end the mission); per-game `Reserving` (CE, H2) vs `SignsIn` (H3/ODST/Reach/H4) - GroundHog gets nothing; each module's feature lists its mission-start hook; `SavedChoice` no longer caches a missing key; one `MapPhase` atomic; module Start moved into each game's graphics hook; CE trail keeps only on-foot samples. Retest caught one bad assumption: MCC's game options `+8` isn't the mode (the struct at `[0xE80A78]` is Halo 2's session object, not the memcpy'd options) - reverted to the session read. H2's count is now written only when the joined players are mapped and others are held back (the old per-tick `min(joined, highest mapped + 1)` could run the count past the mapped players on a fresh load). Box on hj14: CE join/placement/leave/rejoin/lock/deferred save/2-min soak, H2 join placement, H2 fresh 4P Delta Halo (P3/P4 1.8-2.4 from P1), H4 join. **Open:** two MCC "Fatal error!" crashes (AV writing 0x148 at halo2.dll+0x3A76E4, Anniversary renderer, fresh 4P Delta Halo load, hot join on) back to back at 17:45/17:52, then 5 clean runs (3 same build/settings, 1 v1.9.1, 1 hot join off) - not reproduced; the X server was near its 256-client limit (it hit it at 18:35: "Maximum number of clients reached"). Minidumps: scratchpad `crash1745.dmp`, `crash1752.dmp`, parser `dmpinfo.py` (UE4 dumps hold no faulting stack).
- **Codex pass 2** found the H2 fourth join leaving the count at 3 (the "hold only while others are held back" rule) - fixed: the count is written as the joined count whenever every index below it is mapped - and missing-key config reads racing the store's load - fixed: `SavedChoice` reads only after `SplitscreenConfigStore::Loaded()`, then caches hit or miss. Accepted and documented: the roster ints written on the render thread (x86 aligned stores behind a compiler fence, the Players window's existing pattern) and the CE on-foot heuristic (a slow vehicle's path or a jump can pass; worst case a short drop). **Codex pass 3: approve.** Box on the release candidate `WTSAPI32_hj15.dll`: H2 1->2->3->4 (count 4, four views), CE join, H2 fresh 4P Delta Halo (no crash; P3/P4 1.8-2.4 from P1). **Box state:** user's config restored from `alpharing-test/cfg-user-0929` (all five files identical), hj15 installed (md5 2d9c8b73...), vpad.py stopped, MCC quit to desktop.
- **megabitt01 (Discord DM with XiaoDanny, 2026-09-29):** asked to join forces ("a definitive version of AlphaRing that the launcher points to", a hybrid that keeps both designs). The user is open to it: v2.0 first, then PRs to his active branch `master-chief` (his `master` is stale; default `dev` is old). Common ancestor with our main bdad7eb (2026-01-27); his side since: 17 commits, ~100k lines (mostly the dashboard UI); both forks carry XiaoDanny's Reach split and an H2 black-screen fix. Plan: agree on the canonical branch, settings format (his binary vs our JSON) and whether our ImGui overlay/player menus stay beside his dashboard; one integration branch, then PRs by area in dependency order (offsets/hooks, per-game split-screen, Anniversary/Reach/HUD, spawn menus + per-player HUD, Players window + hot join, tooling/CI/docs). The user warned him they'll likely be away for months after this sprint.
- **Box gotcha:** after a day of launches the Steam client leaked ~240 X connections and the X server hit its 256-client limit (MCC couldn't start, screenshots failed: "Maximum number of clients reached"). Fix: kill Steam/steamwebhelper and the Wine processes by name (not `pkill -f '\.exe'` - it kills the SSH command itself), then `mcc.sh launch` cold-starts Steam through EmulationStation.

### 2026-09-27 (part 2) - v1.9.1: Reach split-screen black/invisible Spartans fixed - RELEASED as v1.9.1-experimental (commit 34f710b)

User: "keep debugging it and see if we can find some sort of way to correct this if possible. If not... at least update the Discord... this is our new findings", then "Go" (commit and release it as 1.9.1).

- **Cause:** a Reach engine limit, not AlphaRing (all our Reach hooks off still breaks; the render-quality patch only makes it sooner).
  - With enough bodies and dropped weapons in the world (~440 objects at 4P, ~495 at 2P at The Package), the later split-screen views stop drawing Spartans and first-person weapons, or draw them black or over-bright, last view first.
  - Reach's automatic collector starts only above 120 waiting objects and, for garbage pressure, stops at 115, so the pile sits at that level.
  - Full notes: docs/REVERSE_ENGINEERING.md, "Halo Reach split screen: later views stop drawing objects as bodies pile up".
  - The render limit itself wasn't found. Ruled out: the render-state cache, every Blam data array, and 0x229558.
- **Fix:** `src/mcc/module/entry/haloreach/world.cpp`, a hook on 0x47B76C (the collector's only caller) that sets the two compare immediates before every collection.
  - 0x4FF314 (start) and 0x4FEF10 (stop): 60/55 at 2P, 40/35 at 3-4P, when all players are local (players count == local count).
  - Anything else gets 120/115 back.
  - New offsets `OFFSET_HALOREACH_PF_COLLECT_GARBAGE`, `OFFSET_HALOREACH_TLS_PLAYERS`, `OFFSET_HALOREACH_V_GARBAGE_COLLECT_START/STOP`.
  - It logs "Reach: garbage collection starts above N waiting objects, stops at M" when the limits change.
- **Codex adversarial review, two passes (both acted on):**
  1. The first version (`garbage_collect_unsafe` from the world tick) purged every candidate, including fresh drops, and ignored network play; the docs also wrongly claimed the automatic collector never takes visible objects. Replaced by the native limits and a network guard.
  2. The once-a-second world-tick check could lag a remote join and missed loading-path collections, so the check moved into the collector hook. The docs now say "biggest pile first", not "oldest first".
- **Verified on the box** with a TEMP harness killing P2 (never in git):
  - 4P, 100 deaths: garbage 35-40, all views fine.
  - 2P side by side, 100 deaths: garbage 54-60, P2 fine.
  - Final hook build, 4P, 40 deaths: fine.
- Simplify skill not run: the change is ~30 lines, reviewed twice by Codex.
- Discord: findings posted to #general before the release (that post's "skips anything a player can see" explanation was wrong; corrected in the release post).
- Released on the user's go: tag v1.9.1-experimental, GitHub pre-release with the DLL (https://github.com/kirklandsig/AlphaRing/releases/tag/v1.9.1-experimental), #general announcement with the corrected explanation. SR388 replied: the H2 3-4P Save & exit infinite load happens on an older AlphaRing (360-style menu) with the H2 co-op fix mod - asked him to say if it happens on ours.
- Box restored after testing: v1.9.1 release DLL (sha256 561344ec...), cfg-backup-0926 configs, alpha_ring_patches.cfg removed, vpad stopped, MCC quit from its menu (the Reach resume point is now this test's Package checkpoint).
- Box: Steam wedged after a launch (MCC exited after "PatchConfig::Load"). Killing Steam/Wine by PID restarted EmulationStation too, so the first ES API launch was lost; a second `mcc.sh launch` worked.

### 2026-09-27 - v1.9.0: Halo CE and Halo 2 Anniversary 3-4P, Halo 4 bars-removed HUD, Reach/settings fixes - RELEASED as v1.9.0-experimental (commit f6ed415)

User: "Get that Halo CE anniversary four-player co-op working, as well as the Halo 4 HUD fix", then (2026-09-27, stepping away, autonomous mandate incl. box reboots and the final release): fix salty's and XiaoDanny's Discord reports and add Halo 2 Anniversary 4-player as a surprise.

- **Halo CE Anniversary graphics with 3-4 players** (`src/mcc/module/entry/halo1/anniversary.cpp`, offsets in `offset_halo1.h` "Anniversary graphics (Saber3D)" block; opt-in, experimental). The Saber3D renderer only draws two views, so each frame keeps its stock 2-view frame and the pairs alternate (players 1+2, then 3+4; each view updates at half the frame rate):
  - Quarter-size children beside every `_SPLIT_1/_SPLIT_2` render-target child (`CreateChild` hook on 0x1F9D20, names carry the size so a resize makes new ones), swapped into the parent's +0xA8/+0xB0 for the frame (0x455A10). Cameras get quarter viewports (`QuarterLayout`); cameras 3/4 are added by `SetSplitScreen` (0x4150F0) via AddCamera 0x4684C0 and dropped before the engine drops camera 2.
  - The camera list (0x4547E0 -> append 0x2EA720) gets copies of the pair's cameras in slots 0/1: the append works the aspect out from the view height over the SCREEN width, so the copy's height is scaled and restored after; +0x220 (camera index) is set to the slot because many renderer tables have 2 slots.
  - Composite 0x45E2B0 copies each view into its quadrant of our own full-screen image, which is copied to the screen target after view 1. The classic HUD pass 0x740B0 gets quadrant viewports (hook on the HUD view setup 0xB29438) and draws views 2/3 before view -1; the per-view overlay rect (mid-function hook 0x4513AC) is the quadrant; 3 players: 4th quadrant left black, CE's window layout (0xAC4154) uses 4 quarters when this mode is active.
  - Game side: the per-frame sync 0x89F00 (game thread) hands cameras over (0x89B70; we add views 2/3: window 0xAC4154, window camera 0xAC450C, hand-over 0xB29268, plus the first-person block b29438/0xB27510/0xB275B8), submits first-person models (0x7AC60, views 0/1: for the odd pair the view 2/3 id+node blocks are copied into slots 0/1), and syncs objects. **Timing**: the list built during sync N (on a worker) is committed at the start of sync N+1 and drawn while N+1 runs; state handed over in sync N goes with list N, so the pair is chosen at sync start (`s_pair`) and everything in that sync uses it.
  - **Own-body hiding**: the per-view hidden mask comes from 0xAB2890(object) (local unit 0/1 in first person -> bit 0/1), applied by the object sync 0x783C0 with vt+0x328 = HIDE in view, vt+0x330 = show (not the other way round). Hooked to answer per slot for the pair. The 4-record player sync 0x7D350 handles each player's own first-person-only models (remapped too).
  - Mistakes worth remembering: blanking the local-units table (players +0xC8) around a call broke weapons/HUD because that code runs on worker threads in parallel; module unload leaves pointers into the old halo1.dll -> `EntrySet::on_remove` callbacks (new) reset the state (restarting a mission after quitting crashed with "Fatal error!" before).
  - Setting (CE and H2): `MCC::Splitscreen::AnniversaryQuadChosen/ChooseAnniversaryQuad` (saved `-1.anniversary_quad`), read once per mission into `AnniversaryQuadActive` (the renderer hooks only read that atomic; CE only when the mission starts in Anniversary, H2 whenever chosen since H2 switches graphics in place with Back). UI: Splitscreen > Options menu item and an "ANNIV 3-4P" row on the player menu's MY HUD page (CE/H2 with 3+ players).
  - Box-verified: 4P (all quadrants, own HUD/first-person weapon/scope zoom, bodies of others visible and own hidden, firing, weapon swap, checkpoint revert, pause menu, quit -> resume), 3P (quarters, 4th black), vehicle (Warthog gunner, third person), 60 fps. Not yet: cutscene transitions mid-mission, deaths/respawn, level end.
- **Halo 2 Anniversary graphics with 3-4 players** (`src/mcc/module/entry/halo2/anniversary.cpp`, `CellShaders.h`, offsets in `offset_halo2.h` "Anniversary graphics (Saber3D)" block; opt-in, same setting). Research notes: scratchpad `agent_h2a/plan.md` (not in git). Same alternating pairs as CE, but:
  - Split is the glue's `0x6A4380()==2` test: a PreservingThunk detour on that 12-byte leaf answers 2 at 7 call sites (by return address) in quad mode. Players 3/4 get two camera copies of ours, handed over after view 1 (0x5F510) with our cameras swapped into a temporary copy of the renderer's camera array.
  - The list builder 0x2DDCB0 stamps the pair into the list's flags (bits 28-30, the renderer reads only bits 0-1) so the render thread's frame 0x2DEC00 knows the pair; list_camera 0x1C7740 copies the pair's cameras with an anamorphic aspect (view stays W x H/2, camera shaped like the player's grid cell 0x7E09D0/0x7E0BD0). 3 players: the unused second view is listed with flag 0x4 (skipped by the render loop).
  - **The game's composite quad (0x1D2A00 -> immediate draw 0x194EC0) leaves NO pixels with more than two players** - stock 3-4P Anniversary is black too, retargeted or not (pixel readbacks: view image fine, target untouched; blend/depth/cull/scissor/viewport all benign; not solved). So the quad is collapsed to a point (mid-function hook 0x1D2C33, vertices at rsp+0xC0) and the view's image (PS SRV slot 0) is drawn into the cell with our own shaders (full-screen triangle + linear sample, fxc bytecode in CellShaders.h), saving/restoring the pipeline state. Cells are kept in our texture and the other pair's put back before the UI pass (0x2E3F70), which draws H2's classic HUD for every window.
  - Per view (game thread): each window's visible-object list (0x608A0, stock lists for local 0 / "others") is stored only for the pair's players, in their slots; the own-unit test 0x5FE80 is remapped to the pair; the first-person sync 0x6D0D0 is replaced: builds all players' models (0x81BFB0) and shows each only in its owner's slot (hide vt+0x88 / show vt+0x90 unless part flag 4, flags 8<<slot). `s_sync_pair` is taken at the window loop's first call each game frame.
  - Players with no unit yet (P3/P4 wait for a co-op respawn - missions have 2 starting places; they spawn once the others walk off the respawn spot) draw black, like Classic. MCC's H2 graphics choice is the mission lobby's "Visuals and Audio" (Classic/Remastered), not a settings page.
  - Box-verified: 4P and 3P, first-person weapons and own body per player, zoom per player, Back to Classic and back, pause, Save & Quit -> resume (module reload), checkpoint revert. ~30 fps with 4 live views (~60 on light frames in 3P); Classic 4P is 60.
- **Halo 4 HUD with the black bars removed / Left/Right** (`src/mcc/module/entry/halo4/hud_fit.cpp`): the four ultrawide helpers (0x3DBB8C/0x3DBBEC/0x3DBC40/0x3DBC98) answer for the view being drawn (HUD update 0x3BCF88 / render 0x3F7A7C set the user; UI context +0x28 via 0x3A7BDC elsewhere), edge anchoring forced for split views (0x3FD3E8), the crosshair group moved back (screen render 0x3D6184). `PreservingThunk` now also saves xmm1-5. Box-verified 2P/3P bars removed, side by side, zoom, magnum.
- **Settings**: SplitscreenConfigStore::Load rebuilds all Left/Right table entries (8/9 and 12-14) when the choice is saved without them (found when Reach drew top/bottom with a side-by-side HUD).
- **Discord (2026-09-27)**: answered XiaoDanny (how CE 4P works, H2A likely portable). salty's Reach crosshair ~10% low (3440x1440, 2P/3P side by side, v1.8.0) = MCC's own Settings > Gameplay > Halo: Reach > Crosshair Position "Lowered" (the box had "Centered"); with Lowered, halves and the wide 3P top cell sit ~10% low while quarters stay centred (our v1.8 per-view centring). Told salty to pick Centered. XiaoDanny's Reach 4P black/invisible Spartans + black FP weapon for P3/P4 (his own build, offline, start of The Package after many deaths): NOT reproduced on ours after repeated grenade deaths/respawns and checkpoint reverts there (and 6 kill cycles the session before); asked him to retry with our DLL.
- **Codex adversarial review (fixed)**: (1) CE's CreateChild 0x1F9D20 takes six arguments - the sixth's low word is the texture's flags (+0x7C), the fifth unused; the detour forwarded five, so every CE Anniversary render-target child got a garbage flags word, option on or off (callers pass flags in their [rsp+0x28]). (2) CE applied the quarter layout to non-split lists too (engine's full-screen frames, e.g. cutscenes): quad mode now needs a split list, and leaving it lays cameras out for the list's own split state.
- **Simplify pass**: Halo 4 macro/layout ids/Refresh declaration moved to halo4.h (the macro had been duplicated on one line), HUD fit early-outs (`s_any_fit`/`s_ever_fit`) so single-player and fitting layouts skip the per-frame widget walk and user lookups; H2: return addresses in offset_halo2.h (0x6D0ED dropped - its function is replaced in quad mode), fixed stack array instead of a per-tick vector, own-unit test reads a once-per-frame `s_sync_quad`, render target + kept texture + shaders prepared once per frame (`PrepareTarget`), 3-player flag derived; CE loop/helper cleanups; one `AnniversaryQuadGame` helper.
- Box harness: `vp.sh` now skips pad input when MCC isn't running (twice MCC died on an H2 load and the Steam store came to the front). Two H2 Anniversary load crashes happened before the first frame during the probe builds; none in the ~10 launches after (a fault logger was in for those).
- Box: Steam web helper crash-looped after hard kills (MCC stuck on "PatchConfig::Load"); a clean "Exit Steam" wasn't enough - rebooting the box fixed it. MCC's resolution for 21:9 tests: `GameUserSettings.ini` (compatdata/976730/.../WindowsNoEditor), backup `.bak-v19` restored afterwards.

### 2026-09-26 - v1.8.0: Discord triage - CE spawn fixes, centred split-screen HUD/aim, ODST 4P HUD, species, CE side-by-side zoom/divider/3P - RELEASED as v1.8.0-experimental (commit c778e71)

From the v1.7.0 Discord feedback and the user's own testing with Adonis (box log of 2026-09-25, Ruby's Rebalanced Halo: CE on The Maw):

- **CE "Ally" spawns were enemies on The Maw** (reproduced: an ally Spec Ops Elite shot and killed P1). Ally used team 2 (human), which a CE mission script has to ally with the players' team 1; The Maw (at least in Ruby's mod) doesn't. Ally now takes the spawning player's unit team (object +0x74). Free actors copy their team from the unit's +0x74 (actor attach 0xC027EC); verified ally stays friendly, reticles blue. `src/mcc/spawn/halo1.cpp`.
- **CE chosen weapon ignored** ("Elite with shotgun spawns with a plasma rifle"): the engine refuses a weapon the character can't hold (unit_add_weapon 0xB08F3C fails, the weapon is deleted) and the actor keeps its usual one. Not fixable without animations; the menu now says `Can't use <weapon>: <character>`. Unit weapon slots: unit +0x2D8 (4). New offset OBJECT_TRY_AND_GET 0xB389A4.
- **CE spawns landed beside/behind the player or inside walls**: placement used the body's forward (object +0x30), which lags the look by up to ~90 degrees until the player moves. Now the unit's desired facing (+0x204). OBJECT_GET_ORIENTATION offset removed.
- **Split-screen HUD/crosshair pulled toward the screen centre** (Tuko Mas, two monitors spanning 3840 px): each view's title-safe box (HUD box, crosshair and aim centre) was the view clipped by the whole screen's 5% box. Now each view gets its own 5% inset (same size, centred). H3/ODST: hook the view setup (0x282EC4 / 0x2ABAC8, `LeftRight::ViewSetup`) and the title-safe provider (0x272010 / 0x29F634), answering only at the view setup's call site (`Gen3::title_safe_return`, 0x282F10 / 0x2ABB14). The view setup keeps the screen height in r10 across that leaf call, so the detour goes through `PreservingThunk` (src/mcc/module/entry/PreservingThunk.h, `PreservedEntry`) - a plain C++ detour clipped P1's half. H4 (0x38EE74) and Reach (0x287C5C) have a leaf (slot, players, view*, box*): `LeftRight::ViewportRect` rewrites the box. Box-verified 2P side by side in all four: crosshairs within 1-2 px of each half's centre (stock: 48 px toward the middle). Reach's stored box read back 48/54 px in on every side.
- **Codex finding from the v1.7.0 review fixed**: H3/ODST/H4 black-bar checkboxes are disabled while Left/Right is chosen (as Reach's), so LeftRight's saved table entries can't go stale.
- Checked, not ours: LB = flashlight + grenade (Boxer preset) and B switching grenades are MegaBit-build presets; our defaults were box-checked in CE (LB only switches grenades).
- /simplify pass applied (hook logic moved into LeftRight: `TitleSafe`, `ViewSetup`, `ViewportRect`, `ViewSafeBox`).
- Part 2 (user: "address all of the still open items... then build and push the release"):
  - **ODST 3-4P / side-by-side HUD**: ODST's quarter HUD layouts (resolution 4, 6) carry only the ammo panel; `Gen3::whole_quarter_hud` maps them to the full-screen layouts (0, 2) in `LeftRight::HudResolution`. Box-verified 4P and 2P side by side.
  - **SPECIES row** (PlayerMenu, MY HUD page, H2/H3/Reach): flips `UseEliteModel` on `CGameManager::player_profile(player)`; shown only when the player owns their profile. Box-verified in Reach MP (P2 Elite hands, P1 Spartan); Firefight always makes players Spartans.
  - **P2-4 profile edits kept**: `CGameManager::seed_profile` (zero FOV = never edited) now runs before the overlay editor edits a profile, not only at match start.
  - **Reach 3P loadout (wide slot)**: already fixed by XiaoDanny's loadout port - verified top/bottom and side by side, no change.
  - **H3 4P**: crosshair/aim centred; the rest of the HUD still leans ~33 px inward (was ~58): the quarter layout's own placement (shield sits midway between view and box centre + ~34 px). The HUD's other title-safe caller (0x2DC56C "inward shift", return 0x2DC5D7) was answered with the full screen and changed nothing visible, so it was reverted.
  - **Not fixed (documented)**: H4 HUD with black bars removed stays in the left 3/4 (HUD fit 8:3 and pinned left; the variant-3 surface isn't it - resizing it to full width changed nothing; the host HUD transform (vtable +0x300, H4 site 0x3DD0AC) would move the crosshair too). CE side-by-side zoom distortion (salty): the clear colour shows where the zoomed world isn't drawn - classic renderer, not the Saber `scope_effects` pass (its setters 0x281290/0x281790 never run in classic); `rasterizer_screen_effects` is an unwired stub. Anniversary graphics 3-4P: renderer draws 2 views - separate project.
  - Box harness additions: `setcfg.sh <layout> [patch=1...]` (deploys, sets two_player_layout, strips stale Reach LR entries for 0, writes alpha_ring_patches.cfg; `DLL=` to deploy another build; `PLAYERS=n` sets settings.json player_count), `firefight.sh`. User configs backed up in /userdata/system/alpharing-test/cfg-backup-0926 and restored afterwards.
- Part 3 (user: "fix and resolve and implement everything before we push another release"; nothing was released yet - the earlier "RELEASED" note was premature):
  - **CE/H2 side-by-side divider**: CE's painter 0xB32238 draws a 4 px band across the middle for 2+ views (plus one down it for 3-4) from the window bounds 0x29E05B4, views 0xB321FC, fill 0xAC63F4; the hook paints an upright band (and one across the right half for 3). H2's painter 0x831D90 already has a side-by-side branch keyed on the frame's mode global 0x165C168 (only it reads it; the render entries 0x7E1600/0x7E1990 write it with views 0x165C16C); the hook presents mode 2 around the call. Box-verified 2P and 3P.
  - **CE 3P side by side**: window builder 0xAC4154 (view, views, rect, copy) - view 0 of 3 is built as view 0 of 2 (the 2x1 grid), views 1/2 as views 1/3 of 4 (right quarters). `Supports(Halo1)` now 2-3. Box-verified.
  - **CE side-by-side zoom (salty)**: MCC's scope-effect texture-transform builder 0xAC7168 (gated on the pistol/sniper scope masks, views > 1, even pass) hard-codes the stock grid into four floats at [rsp+0x40/0x44/0x48/0x64] (screen size in views, minus the view's column/row). A mid-function hook at 0xAC73DC (`MidFunctionThunk`, PreservingThunk.h) rewrites them from the view's window (0x29E05B4) and the screen (0x1B7D3DC) and reloads xmm15 (holds the row). Stock layouts give MCC's own values. Box-verified pistol 2x, 2P and 3P.
  - **H3/ODST HUD lean in quarters**: chud_draw_begin (H3 0x2ED0D4) clamps the HUD pixel frame (0xAD3140) to the HUD record's global safe frame (H3 +0x28/+0x2C, ODST +0xAC/+0xB0; ~87% of the whole screen, centred) in screen space, trimming a quarter only on its outer sides. `HudLayout` now returns a copy with it zeroed for split views (the game skips the clamp at 0). H3 4P frame now symmetric [34,56,506,904], shield bars at 480/1440; H3 side by side 2P/3P and ODST 4P re-checked. Tried first, no visible effect, reverted: the split HUD's inward shift 0x2DC56C (6 calls at widget creation) and answering the render-view init's title-safe call (0x2A6234, return 0x2A6265).
  - **H4 side by side**: the HUD layout picker 0x3BD78C (variant 0 full 0x80076, 1/3 half 0x80077, 2 quarter 0x80078) now returns the quarter layout for Left/Right's full-height halves - nothing cut off; the tracker sits mid-height (layout 360 units in a 720-unit view). Box-verified 2P and 3P.
  - **Settings data loss (found in testing)**: Profile::Save opened settings.json (truncating it), then `j.dump(4)` threw on invalid UTF-8 - a seeded P2 profile's GameSpecific (0x100 bytes at 0x310) holds the game's runtime pointers - leaving the file empty. All six JSON writers now go through `WriteJson` (dump with the replace error handler, write `<file>.tmp`, rename over), and GameSpecific is neither saved nor loaded. Box-verified: overlay Save Profile and the SPECIES row with an unseeded P2 (FOV 0 -> seeded 70).
  - PlayerMenu MY HUD rows shrink to fit a half-height view (6 rows with SPECIES ran into the hint line).
  - /simplify pass (4 agents): shared `LeftRight::Rect` and `PaintBands` (Gen3 + CE dividers), `EmitCode` for both thunks, Gen3 HUD-record fields grouped (hud_safe_frame after hud_canvas), CE window hook remaps then calls once. Codex adversarial review: `WriteJson` now closes and checks the stream before the rename (a failed flush could replace settings.json with partial JSON). Codex bug hunt: SPECIES hidden without the split-screen override (MCC's own profiles then; the flip wasn't saved). Box re-checked after the refactor (CE zoom/divider, H3 3P dividers, saves); box configs restored from cfg-backup-0926, final DLL deployed (md5 7ad6610f...).
  - **Not done (documented)**:
    - H4 HUD with black bars removed: the four screen-wide ultrawide helpers (0x3DBB8C wide?, 0x3DBBEC ratio(cl), 0x3DBC40 extra x, 0x3DBC98 extra y) poked per view (wide, 1.333, 160, 0) plus the per-element anchoring gates forced (0x3FD4B6 nop, 0x3FD4DF jmp) spread the HUD correctly and fix nametags (0x3F25BC), but every CUI screen's root translation (0x3D6184 at 0x3D61EB; no-translate flags screen +0x662/+0x663) also moves the crosshair: it lives in sub-screen id -0x12DE0D0 (reticle container vt 0xD6C2D0, widget at 640,360 in 1280x720 units) and an x offset (+0xC0) of -extra re-centres it. The zoomed scope mask is still lopsided, menus share the same helpers (need per-screen context: hook 0x3D6184 and gate on HUD layout ids), and 3P needs per-user values. Diagnostic code kept in scratchpad h4_diag_splitscreen.cpp.
    - Anniversary 3-4P (static RE by a subagent): both Saber renderers hard-code two stacked views - CE camera creation 0x4684C0 (once), per-frame camera list 0x4547E0 (two), layout 0x4F2120 (height x0.5), render-target children "_SPLIT_1/2" (tex+0xA8/+0xB0, 0x1F3E90/0x1F9BD0); H2 camera builder 0x2DDCB0, viewport 0x2CF100, player->viewport 0x4D610 (0/1 only), "%s_split0/1" targets 0xF7750. Estimated weeks.
  - Box: MCC stopped launching after many relaunches (Steam in a bad state, store window up); killing all Steam/Wine processes by PID and cold-starting via mcc.sh fixed it (as in the 2026-09-25 note).

### 2026-09-25 (part 2) - v1.7.0 side-by-side split in every game - RELEASED as v1.7.0-experimental (commit c02ecc6)

User asked: "Yeah, do it for all of them" (vertical split beyond Reach, toggleable, saved), plus "is dual wielding broken in Halo 2 and 3?" and whether MegaBit/XiaoDanny already did the other games (no - fork survey: only XiaoDanny's Reach #20; adamdavies1915 has an unimplemented H3 table-only plan).

**Design:** `src/mcc/splitscreen/LeftRight.{h,cpp}` - one saved choice (the store's `TwoPlayerLayout`, still in `alpha_ring_splitscreen.cfg`), `Supports(game, players)` (CE: 2, H2/H3/ODST/H4/Reach: 2-3), `Choose()` (Reach store + re-apply Reach patches). Gen3 engines (H3/ODST/H4) described by a `LeftRight::Gen3` (table, screen, player count, fill-rect, pool release/init, rounding, HUD record size/canvas offset); per-game hooks in `module/entry/<game>/splitscreen.cpp`. The store now exposes `LayoutEntry`/`LeftRightEntry` and `Load()` runs for every module (before hooks) so the pool is built for the saved choice.

**Gen3 mechanics (H3 verified in depth, ODST/H4 by signature):**
- Table writes each frame from the game's render hook (H3 0x18553C; ODST render.cpp hook; H4 0x12259C in its own entry set), stock/patched entries snapshotted and restored on Top/Bottom.
- Divider painter replaced (H3 0x2D8174 / ODST 0x303C54 / H4 0x3C66D4 which needs setup 0x34D224(0,1) + 0x34D14C(0) - first H4 run crashed with setup1(0) only).
- Render target: variant 3 is the half surface. H3 0x2757C8 / ODST 0x2A2C64 create hooks resize the pair at sizes+0x18/+0x04 to (w/2, h), recomputing the unshrunk size from the descriptor (H3 double rounding, ODST floorf) and only when it reproduces the pool's 3/4 x 1/2 result. H4 sizes variants in a helper (0x37E8B4): the hook asks it again for variant 0 and halves the width.
- Live switch: layout generation changes -> the engine's own pool release/init pair (H3 0x27613C/0x275BBC, ODST 0x2A3400/0x2A30F0, H4 0x37F610/0x37F3A8) from the render hook, as its resize path does (minus ResizeBuffers). Verified both directions in H3 and H4.
- HUD (H3/ODST): `HudResolution` (H3 0x2F1E38 / ODST 0x32DD0C: 1/5 = two-player half, 4/6 = quarter) maps halves to the quarter layout; `HudLayout` (H3 0x2ECF38, record 0x64, canvas +0x10 / ODST 0x3284B4, record 0x110, canvas +0x94) returns a per-user copy with canvas height = width x viewH/viewW. Round motion tracker at the bottom verified.
- Aim: H3/ODST build each view's projection off-axis around the centre of its title-safe box (0x2A63E4 reads the safe rect at view+0x48) and the crosshair follows, so both sit ~47px toward the screen centre in a half - stock behaviour (same in their 4P quarters); centring only the HUD box was tried and reverted (it moved the HUD away from the crosshair).
- H4 HUD NOT adapted: it keeps the stock two-player HUD pixel layout from the view's top-left (lower half unused, top-right weapon panel clipped). Its canvas isn't in globals (diffed .data, scanned rect globals and the process for chud records) - open.

**H2 / CE:** H2's window grid (0x7E09D0 / cell 0x7E0BD0) has a native mode: MCC passes 1 (rows first); any other mode = columns first, 3P player 1 full-height left - hooks pass 2. CE's grid (0xAC4108) always grows rows first - hook returns 2x1 for 2 players (3P stays stock). Both need Classic graphics (Saber renderer draws stacked views), so `ClassicGraphicsScope` (mission start) forces Classic and takes the choice for the mission (`StartClassicMission`); mid-mission changes wait (menu status says so). Known: the stock 2px (H2) / 4px (CE) divider line still crosses the screen - CE's composite is one full-screen quad, so it's drawn inside the game frame; painter not found.

**Menu/UI:** SPLIT row on the MY HUD page (everyone's setting); the menu now opens in H4 with only SPLIT ("SCREEN" page); ViewRect follows `OnScreen`. Overlay: Splitscreen -> Options -> Side-by-side split; the Reach Dev Tools combo now routes through `Choose()`.

**Dual wield:** our defaults already matched MegaBit's current upstream (Swap/Reload Left = Reload Right = RB; his Y variant was older). Real gap: mappings saved before 1.6.0 (settings.json profiles, custom_mappings.json - the box's "8bitdotemp" profile has them -1) had all shared actions unbound. `kSharedActions` table now drives defaults, fills legacy mappings (only when all six are unbound) and rebinding (shared actions still on the old button follow). Not verified in game (harness couldn't get a dual-wield prompt up).

**Reviews:** /simplify (4 agents) applied - CPatch::apply reuse, patch-set re-apply, Supports() table, typed saved entries, menu rows, comments. Skipped: folding Reach's own implementation into Gen3 (would change XiaoDanny's tested behaviour). Codex adversarial review: fixed CE/H2 live switch bypassing Classic (now mission-start), FillSharedActions clobbering deliberate None, rebinding clobbering custom shared bindings. Second Codex pass failed (Codex credentials 401).

**Box:** Steam leaked ~240 X connections over the day (MCC "crashed" at startup) - restarting Steam via the ES API fixed it (kill by PID, never type the s-word remotely).

### 2026-09-25 - v1.6.0 all-inclusive update - RELEASED as v1.6.0-experimental (commit 49afa5f)

User asked for: integrate XiaoDanny's Reach vertical split (megabitt01 PRs #17/#20) with full credit; full per-player HUD customization (positions, colours, presets by monitor type - a tester's dual-monitor photo showed H2's HUD bunched in the middle); fix what the other forks/issues reveal; armed AI with a per-spawn weapon choice (H3 AI spawned unarmed and meleed); then a Codex adversarial review; report back before pushing.

**Reach port (XiaoDanny, verbatim + credit headers):** `module/entry/haloreach/*` (vertical split render targets, HUD anchor, CUI canvas fit, FOV baseline, loadout fix, black bars), `PatchConfig`, `SplitscreenConfigStore`, `DebugFlags.h`, `docs/REVERSE_ENGINEERING.md`, Dev Tools window (Module.cpp). Dropped his two diagnostic probes (`chud_const.cpp`, `res_path.cpp`: hot-path hooks with compile-time-off flags). Our fixes on top: `PatchConfig::Load()` was never called (saved states ignored); `CPatchSet::hModule` uninitialized; write-list mutex in `SplitscreenConfigStore` (Codex). Verified: 2P Left/Right, 3P L/R, per-slot FOV logs, 4P Reach HUD placement.

**Per-player HUD (`src/mcc/hud/`, `CGameManagerHud.cpp`, `module/entry/halo1/hud.cpp`):** MCC's host vtable has per-element HUD callbacks every game asks while drawing (`+0x2F8` anchor, `+0x300` transform dx/dy/scale, `+0x308` colour) - answering them for the drawing player gives per-player offsets/scale/hide/colour in H2/H3/ODST/Reach (CE adds host offsets to X only, so CE offsets come from a hook on `hud_calculate_point` 0xB56A58). Per-game table in Hud.cpp: drawing-user global, MCC element id -> ours, units (gen3 virtual canvas / H2 pixels x unit scale / CE none), the game's own HUD box (CE = centered 4:3) and each element's side. **HUD area presets** (Game default / Screen edges / 21:9 / 16:9 / 4:3) shift left/right elements so the HUD's sides land on a centered box of that shape - verified in all five games at 32:9 quadrants; CE "Screen edges" spreads its 4:3 HUD to the view edges. Controller "MY HUD" page (area/size/colour/reset) in the D-pad menu; in Reach (no spawn backend) the menu opens straight on it. Offsets in `offset_halo*.h` under "per-player HUD".

**H2 ultrawide bunching (the tester's photo) root-caused + fixed:** with the profile's `LockMaxAspectRatio` ("HUD anchor: Centered", global in H2 at 0x197EE40) and a view wider than 16:9, `0x954DD0` insets H2's whole-screen HUD frame to a centered 16:9 box and `0x7E0A40` splits that among players -> HUDs bunched to the middle, crosshairs off-center. Patch `halo2+0x954DF2` 74->EB ("HUD at screen edges", default on). Before/after verified live with the flag set (doc/images/h2-ultrawide-*.jpg). H3 is different: its `chud_draw_begin` (0x2ED0D4) insets per player's own view with a per-player flag, so no bunching.

**Armed AI (`src/mcc/spawn/`):** H2/H3/ODST squads arm actors only from the scenario weapon palette (weapon index -1 = unarmed; verified in H3 0x55D32C and H2 0x621AB0). The borrowed spawn point gets a palette index; a weapon missing from the palette borrows the last palette entry (`ScopedPoke` + `WeaponPaletteIndex` in Backend.h) for the duration of the synchronous `ai_place` (H3 path 0x577AA4 -> ... -> 0x55D32C arms the actor). "Their usual weapon" = the weapon the mission's own squads give that character most (counted once per map in gen3), else the character tag's first carriable weapons-properties entry. CE swaps the actor variant's weapon tag (actv+0x70) around `actor_customize_unit`. `MountedWeapon()` hides turret/vehicle/character-built/`_integrated` weapons. UI: LT/RT on the Characters page; Weapon combo in the F4 Spawn window. Verified by a temporary in-DLL readout of unit inventories (H3 unit+0x268, ODST +0x27C, H2 +0x22C): H3 Marine BR / Brute Spike Rifle / chosen weapons; ODST Brute Spike Rifle, Brute Captain Automag; H2 Elite Plasma Rifle, Marine chosen Plasma Rifle, Grunt Plasma Pistol; CE sniper swap. Fault safety: handlers record temporary game-data changes (`Command::RecordChange`) and the dispatcher's SEH handler restores them (Codex: /EHsc skips destructors on structured exceptions).

**Fixes from the fork/issue survey (scratchpad research_repos.md):** players 2-4 got zeroed container profiles by default (no sound, FOV/sensitivity/HUD scale 0) -> seeded from player 1's real profile when FOVSetting==0; look sensitivity bytes (0x1B5/0x1B6) were bools; default dual-wield/vehicle bindings (MegaBit 1.3.5/1.3.6); CE level-end freeze candidate (all pad slots answer "connected, released" while a CE map loads - same as the overlay-open workaround; **untested**, needs a level end); version mismatch now disables the mod (assertm is compiled out of Release!); XInput LoadLibrary fallback; no console under Wine; overlay scale from screen size; `_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR`; ImGui WndProc only fed while the overlay is shown (+ ClearInputKeys on hide); 3-player and Reach L/R menu placement.

**Patches:** `CPatch` now captures the module's original bytes once when the module loads (`CPatchSet::update`), guarded against unreadable patch.xml offsets; disabling re-applies the other enabled patches (overlaps). Fixes "a default-on patch saved as off got applied anyway".

**Reviews:** /simplify (4 agents) applied; Codex adversarial review (plugin `adversarial-review` + a rescue bug-hunt): fixed SEH restore, Reach L/R menus, patch capture safety/overlap, usual-weapon counting, H2 weapon choice before squad edits, overlay stuck keys, Reach write-list race. Not fixed: HUD settings read without a lock while being edited (plain floats/bools; worst case one frame of mixed settings).

**Observed, not ours:** one H2 load crash in the Saber renderer (halo2+0x3A76E4, null `[rbx+0x30]`, only halo2 frames on the stack) - did not reproduce on retry. H2/CE players 3/4 sometimes spawn outside the map (black view) - known engine limit, co-op mods.

### 2026-09-24 (part 2) - Spawn menus + per-player controller menus - RELEASED as v1.5.0-experimental

User asked for a prettier/more intuitive menu and a spawn menu (vehicles, weapons, enemies/friendlies) for at least CE, H2, H3 (+ ODST), and for **each player to open their own menu in their own quadrant with D-pad Down**. All done and verified live on the Batocera box with 4 virtual pads (see "Test harness" below).

**Architecture (`src/mcc/spawn/`):**
- `Command.cpp/.h` - fixed, code-registered handlers (`spawn_list`, `spawn`) run on each game's own thread. H3/ODST: a scheduler on the World tick hook (`World::AddTask`; engine calls from any other thread fault in thread-local state). CE/H2: MCC's `execute_command("HS: @ar ...")` reaches the game's console-script compiler, hooked in `module/entry/halo1|halo2/console.cpp`. `Schedule` drops tasks queued before another map loaded (`CGameManager::load_generation`). **Security constraint from the user's auto-mode classifier: never add generic engine-call/memory-read commands or file-driven command channels** - handlers stay fixed and are posted only by the overlay.
- `Spawn.cpp/.h` - shared catalog (per game + load generation), per-player status lines, the F4 "Spawn" window. `PlayerMenu.cpp` - per-player menus: `HandlePlayerInput` is called from `CGameManager::get_key_state` with each player's pad (the pad is zeroed for the game while the menu is open, and the buttons that close it are held back until released); `RenderPlayerMenus` acts/draws on the render thread (ImGui now runs a frame when a player menu is open even with the overlay hidden). Button configurable: `spawn_menu_controller=` in `alpha_ring_menu.cfg` (default DPAD_DOWN, NONE = off).
- Backends: `halo1.cpp` (CE: tag iterator + `object_new`; characters = actor variant's unit + `actor_customize_unit` + `ai_attach_free`), `halo2.cpp`, `gen3.cpp` (H3 + ODST share code; `Game` struct with per-game addresses/layout, `single_locations` = ODST squad layout). Objects: `object_placement_data_new` + `object_new` (+ post-create in gen3). **Characters (H2/H3/ODST): squad hijack** - borrow a spawn point of an EMPTY squad nearest the player, rewrite squad/fire-team/location (team, nearest zone, no objective/scripts, character = palette index, position/facing), call the engine's `ai_place` worker on that one point, restore the bytes (H3/ODST after 60 ticks via the scheduler, H2 immediately - H2 places synchronously). ODST places single locations only through their designer cell (cell -1 = skipped by `0x610AE4`), so the cell is borrowed and its weapon lists emptied.
- Catalog filters: H3/ODST list only tags in the loaded zone set (engine bit test `TAG_LOADED`, the check `object_new` itself makes); mounted turrets, `objects\levels\` set pieces and H2 `scenarios\` parts are hidden; characters come from the scenario character palette (deduped). Names are prettified (`DisplayName`: "brute_captain" -> "Brute Captain", "smg" -> "SMG").
- All offsets live in `lib/game/inc/1.3528.0.0/offset_halo{1,2,3,3odst}.h` under "spawning". ODST addresses were mapped from H3 by byte signature / call order (`map_odst.py` in the harness); H2's from its HaloScript evaluators (`players`, `objects_distance_to_position`, `object_create`, `ai_place`, `ai_living_count`) and live memory (tag-name table at `0x15E4B68/78` next to the Assembly RTE cache globals). ODST's per-thread globals are shuffled vs H3, so gen3 finds data arrays ("players", "object", "squad") by name in the module's TLS block.

**Root-caused + fixed the "second Halo 3 load hangs" bug** (was blocking): MCC loads ALL game DLLs at the main menu and unloads all but the chosen one, then reloads them at new addresses. AlphaRing never removed MinHook hooks on unload, so `Entry::update` -> `MH_RemoveHook(old target)` wrote ODST's saved prologue bytes into the NEW halo3.dll mapped over ODST's old range (the hang site halo3+0xD90C1 is next to ODST's world hook +0x109F78 landing at halo3+0xD9F78). Wiring the ODST entry set exposed it. Fix: `EntrySet::remove()` in `CModule::unload_module` (while still mapped) + `CPatch::apply` refuses to write while the module isn't loaded. Bisected with `h3twice.sh`.

**Overlay:** Halo-style dark theme, fonts that exist under Proton (msyh.ttf/arial - the old code only looked for msyh.ttc and fell back to ProggyClean on Batocera), bold 34px menu font, font atlas built at boot, a "Home" panel shown on first open (replaces "Tutorial").

**Verified live (4 virtual pads, final build after /simplify):** CE (Warthog + enemy Elite that killed P1), H2 (Ghost/Warthog, enemy Elite/Grunt killed by marines, ally Elite/Marine alive), H3 (Warthog, Battle Rifle, enemy Grunt, ally Marine; double-load test passes), ODST (4 menus at once, Grunt/Assault Rifle/SMG Ammo/Banshee from 4 players simultaneously, ally Marine). Player input is blocked while their menu is open (right stick held 1.5 s -> view unchanged).

**Test harness** (Windows side, contains the box password - NOT in git): `Projects/Batocera/alpharing-harness/` (README inside: `deploy.sh N`, `g.sh`, `click.sh`, `h3twice.sh`, disassembly helpers, HS tables, Assembly scnr plugins). Box side: `/userdata/system/alpharing-test/`. MangoHud (forced by Batocera's Steam launcher) hides with Right Shift + F12 held.

### 2026-09-24

**4-player splitscreen made to work in every MCC campaign — live-tested on the Batocera box (Proton); shipped in v1.5.0-experimental**

Test setup (box = Batocera, MCC under Steam/Proton Experimental, identical game build 1.3528.0.0): all patch/hook offsets were first verified by disassembling every patch site in the game DLLs (they're correct for this build). Then each campaign was driven remotely with 4 virtual XInput pads (python-evdev uinput, harness in `/userdata/system/alpharing-test/` on the box) and checked with screenshots + per-quadrant input diffs.

Results (final build, 4 players, 60 FPS):
| Game | Result | How |
|------|--------|-----|
| Halo 3 | ✅ 4 views, 4 independent pads | worked out of the box |
| Halo 3: ODST | ✅ | worked out of the box |
| Halo Reach | ✅ | worked out of the box |
| Halo 4 | ✅ | NEW deferred join (see fix 2) — before: all views black with 3-4 players |
| Halo CE | ✅ | NEW forced Classic graphics (fix 3) + Workshop mod "Halo CE 3/4 Player Co-Op Fixes" (3686670451) for spawns/cutscenes; built-in CE also shows 4 views now |
| Halo 2 Anniversary | ✅ (Classic) | NEW forced Classic graphics (fix 3) + Workshop mod "Halo 2 3/4 Player Co-Op Fixes" (3730810482); without the mod P3/P4 only spawn after a co-op respawn |

Fixes (all in working tree, uncommitted):
1. **Boot hang under Proton** (`Window.cpp`, `Global.h`): overlay was shown at boot and the WndProc returned `true` for *every* message while `WantCaptureMouse` was set; under Proton the cursor starts at 0,0 over the ImGui menu bar → the game's message pump starved → MCC hung at the first frame. Now only mouse/keyboard messages ImGui actually wants are swallowed, and the overlay starts hidden (F4 / Back+Start opens it).
2. **Halo 4 black screen with 3-4 players** (`CGameManagerSplitscreen.cpp`, `CGameManager.cpp/.h`): H4 renders black if a mission *starts* with >2 local players, but joins late players fine. `get_xbox_user_id` now reports 2 players for Halo 4 until 3 s after the game-state `Running` event, then all; the session clock resets on `game_restart` (end of session). Log line `Splitscreen: exposing N local players` shows the switch.
3. **CE / H2A: only 2-3 views with Anniversary graphics** (new `src/mcc/module/entry/halo1/graphics.cpp`, `halo2/*`, `ClassicGraphicsScope` in `Splitscreen.h/.cpp`, offsets `OFFSET_HALO1_PF_GAME_START 0x939D0`, `OFFSET_HALO2_PF_COPY_GAME_OPTIONS 0x39CE0`): bit 0 of game-options byte 0 = Anniversary visuals in both engines (found by live memory diffing). With 3+ players the bit is cleared only while the engine copies the options, so sessions start in Classic; MCC's own setting is untouched. Halo1/Halo2 CModules now get real EntrySets. (Deferring joins does NOT work for CE/H2 — their engines never pick up late joiners; verified.)
4. Review pass (/simplify): `eState::Running`, helpers private, `std::atomic` state, typed Halo2Entry typedef. Removed unused `halo1/render.cpp` hook.

Known issue (open): **Halo 2 co-op mod + 2 or more players: Save & Quit (and overlay Exit Game) hangs on the MCC loading screen** — the H2 engine thread ends but MCC never gets the exit states. Vanilla single-player with the mod quits fine, built-in H2 with 4 players quits fine, Restart Mission works. Progress is autosaved before the hang. Workaround: quit MCC with the Batocera exit hotkey. Not investigated further.

Diagnostics used (not in code any more): temporary `RSSetViewports` probe logged per-frame viewports (showed H2 drawing 4 quadrant viewports while the 4th stayed black → renderer, not player count). Upstream megabitt01 has since shipped 1.3.4-1.3.8 (black-bar/loadout fixes, H2 "black screen" composite-viewport fix, Reach vertical split) — none were needed for the above; see PR #17/#20/#23 if H2A Anniversary-mode splitscreen is wanted later.

### 2026-07-17

**Full audit session: upstream comparison + deep code review + fixes — RELEASED as v1.4.5-experimental (tag + GitHub release with DLL, same day)**

**Upstream intelligence (MegaBit = Jack Bittner = thejackbitt = megabitt01 — same person, our upstream):**
- His active line is `megabit/master`, tag `1.3.3` "beta" (July 6, 2026). His April 2026 commit literally ports OUR changes back ("feat: porting over changes from kirklandsig fork") and his README credits this fork — the COM-leak/MessageBox/GetFunc fixes in his tree ARE ours.
- His new work: a full Xbox-360-dashboard-style UI (~95k lines, embedded Segoe UI font + nav sounds, SDL2/vcpkg deps) replacing the JSON settings system with a binary format — a big identity/dependency decision, NOT ported.
- Portable ideas from his master: `MenuConfig` configurable hotkeys (PORTED this session, credited), Halo Reach armor-staleness fix in `get_player_profile` (NOT ported — his version force-syncs all players' armor from player 0, which would clobber our per-player armor presets; needs adaptation if Reach armor staleness is reported), per-game color-index mapping for CE/H2 (`colorMapping.csv`, `CXboxColorMapping`) — possibly explains wrong colors in CE/H2 when applying our profile presets; investigate if reported.
- His documented Halo CE stutter root-cause (blocking file I/O in `game_setup`/`game_restart` hooks): our hooks are thin, we are NOT affected.
- The `megabit/dev` branch is WinterSquire's separate manager-based rewrite (Mar 2025): its hand-rolled hook engine is WEAKER than our MinHook (no Jcc/RIP-relative relocation, no thread suspension) — do not port; its dollycam is half-wired with an infinite-loop bug; its profile handling is behind ours. Only ideas worth stealing: named critical sections for render-vs-game thread state, declarative per-game patch tables, `signal_end_frame`-preferred overlay rendering.
- Remote `megabit` added locally (`git remote -v`) for future comparisons.

**Fixes applied (see Known Issues for the crash root-cause details):**
1. `CUserProfile.cpp` — ServiceTag `%ls` overread fixed (LIKELY the long-standing profile-save crash); edit box now clears stale tail chars
2. `Settings.cpp` — 4-char ServiceTag no longer truncated to 3 on load (2 sites); profile `name` load now bounded + always terminated
3. `Log.h` — LOG_* macros null-guard `default_logger` (no crash if log file unwritable)
4. `Input.cpp` — `GetXInputGetState()` now internally skips disconnected slots via a 500ms-cached connected mask (and still zeroes the out-state), so every per-frame poll (game input path, bind flows, menu combo) avoids the well-known ms-scale `XInputGetState` stall on empty slots without per-call-site guards
5. Ported MegaBit's `MenuConfig` (credited in source): `alpha_ring_menu.cfg` (next to the game exe, like settings.json) lets users rebind menu hotkey/combo; defaults preserve F4 + START+BACK
5b. New `String::fixedToNarrow`/`narrowToFixed` helpers in `lib/utils/src/String.h` own the "fixed-width non-terminated wchar field" contract (used by ServiceTag UI + both load paths; older save-path sites still use the inline tagCopy pattern — candidate cleanup)
6. Hygiene: stray `nul` file deleted; `dep/` + `.claude/` gitignored (build uses tracked prebuilt `lib/`, `dep/` is vestigial)

**Verified:** MCC 1.3528.0.0 installed locally matches `GAME_VERSION` — build compiles clean (`build/Release/WTSAPI32.dll`).

**Codex adversarial review round (same day, all findings verified in code then fixed):**
1. `Settings.cpp` `Splitscreen::Save` truncated settings.json to just the splitscreen section, deterministically ERASING saved profiles ("profile_t") on any option toggle — pre-existing data-loss bug, now read-modify-write like `Profile::Save`
2. `Splitscreen::Load`/`Profile::Load` let nlohmann parse/type exceptions escape (corrupted settings.json = abort at init) and applied `player_count` unclamped (>4 → `ProfileContext` dereferenced null `get_profile`) — now try/caught, clamped 1-4, and null-guarded
3. `Log::Init` hard-failed (and `main.cpp` assert aborted the game) when the log file couldn't be created — now best-effort: file sink, else console-only, else null logger; never fails
4. Menu hotkey leaked into the game (bind SPACE → menu toggle also jumps) and autorepeated while held — WndProc now consumes trigger keydown+keyup, toggles only on initial press (lParam bit 30), but lets the key type when an overlay text field is focused
5. Controller mask refresh probed all empty slots in one burst every 500ms on a hot thread — now event-driven: `WM_DEVICECHANGE` → immediate full rescan, otherwise max ONE empty-slot probe per 500ms (round-robin), plus instant mask drop when a connected pad's poll fails
6. `MenuConfig` chord parsing failed open (typo `START+BAKC` → bare START binding) — now rejects the whole chord on any unknown token and logs a warning

### 2026-02-05

**v1.4.4-experimental released** based on user's alpharing.log from v1.4.3

**Analysis from log:**
- Crash was in `Profile::Save()` (the "Save Profile" button), NOT in `CustomProfile::SaveProfile()` (preset save)
- Same `wcstombs` buffer overflow issue we fixed in v1.4.2, but in a different code path

**Fixes:**
- Added null-termination safety for `ServiceTag` and `LoadoutSlots[].Name` in `Profile::Save()`
- Added null check in `CaptureFromRuntime()` for invalid profile pointers
- Added logging to both functions

**Commit:** `9a41c55` - fix: wcstombs null-termination in Profile::Save (the actual crash location)

**Status:** User still having issues on v1.4.4. Waiting for new log.

### 2026-02-04

**Issue reported:** User on Windows crashes when: "Use player1's profile" → Apply Profile → Save Profile

**Completed:**
1. **v1.4.2-experimental** - Added null-termination safety in preset save path
2. **v1.4.3-experimental** - Added comprehensive file logging system

**Commits:**
- `bbb06fa` - fix: prevent crash when saving profile preset with non-null-terminated strings
- `3f1f23b` - feat: add comprehensive file logging for crash debugging

**Status:** User still crashing on v1.4.2. Got log from v1.4.3 → led to v1.4.4 fix.

### 2026-02-02

**Completed:**
1. **Fixed profile save/load bugs** - Arrays (Skins, LoadoutSlots, etc.) now properly captured and saved
2. **Fixed missing fields** - DialogueColorStyleSetting, SubtitleSetting, MouseAircraftControlsInverted
3. **Added null checks** - Prevents crashes when loading incomplete JSON files
4. **Added custom profile presets** - Save/load armor, colors, sensitivities as named presets
5. **Created v1.4.1-experimental release** - Includes all bug fixes and new feature

**Commits:**
- `ad977bf` - fix: profile save/load bugs and missing field captures
- `b78d49a` - feat: add custom profile presets (armor, colors, sensitivities)

### 2026-02-01

**Completed:**
1. Fixed menu navigation bug
2. Added custom mapping profiles
3. Added Reset to Defaults button
4. Fixed Proton compatibility
5. Added Steam Input documentation
6. Created v1.4.0-experimental release

---

## Next Steps

000000. **(2026-09-30) v2.0.1-experimental released** on the user's go (commit ccd9127, screenshots b469ae9,
https://github.com/kirklandsig/AlphaRing/releases/tag/v2.0.1-experimental, asset = `scratchpad/WTSAPI32_fixE.dll`,
md5 fa093a66...). Issues #2 #3 #4 #5 #7: status lines updated, cause/fix comments posted, closed as completed (#7
retitled: "drawn big and stretched", not "zoomed out"; #4/#5 got an update note under their first theories). The
v2.0.0 release page now has the v2.0 screenshots (Players window, Splitscreen > Options, hot join, HUD window, spawn
menus) and the issue list marked fixed. Not announced: the user posts on Discord themselves (a casual "fixed these
quick" message, drafted in chat). The box runs the v2.0.1 DLL with the user's config. Open: #6 H2A 1P black (no repro),
the H2A effect readers (minor), H4 4-player switch-off path untested. Next: merging with MegaBit's builds (plan in
`docs/UPSTREAM_SYNC.md`, local and out of git - it holds his Discord answer and our drafts).
00000. **(2026-09-29) Upstream sync with megabitt01: plan in `docs/UPSTREAM_SYNC.md`.** Next: the user sends the draft
message (or approves posting it); on MegaBit's answers, do "Before PR 1" (vcpkg build of `master-chief`, box
baseline, run #24), then the PRs in order, each on the user's go.
0000. **(2026-09-29) v2.0.0-experimental released** (commit 63ca81a, https://github.com/kirklandsig/AlphaRing/releases/tag/v2.0.0-experimental; announced on #general 19:36). Release asset = the box-tested build (md5 2d9c8b73..., scratchpad `WTSAPI32_hj15.dll`), installed on the box with the user's config. Open: the two unexplained H2 Anniversary-renderer crashes on fresh 4P loads (see the v2.0 entry); Belle (Discord, Windows 11) reports Halo CE "fatal error" when a player first picks up a new weapon (sniper, rocket launcher, shotgun), fine after a restart - unexamined. Next: megabitt01's "join forces" plan (PRs to his `master-chief`, see the v2.0 entry).
000. **(2026-09-27) v1.9.1-experimental released** (commit 34f710b, https://github.com/kirklandsig/AlphaRing/releases/tag/v1.9.1-experimental; announced on #general) - Reach split-screen black/invisible Spartans fixed; see the 2026-09-27 part 2 entry. Still open: salty's Reach 21:9 side-by-side HUD asymmetry, Reach 2P top/bottom bottom-view HUD squeeze, SR388's H2 3-4P Save & exit infinite load (older AlphaRing + co-op fix mod; unknown on ours).
000. **(2026-09-27) v1.9.0-experimental released** (commit f6ed415, https://github.com/kirklandsig/AlphaRing/releases/tag/v1.9.0-experimental; announced on the AlphaRing Discord #general; published autonomously under the user's standing v1.9 mandate) - see the 2026-09-27 entry. Open: XiaoDanny's Reach black/invisible Spartans fixed in v1.9.1 (see the 2026-09-27 part 2 entry).
Also seen: in Reach 2P top/bottom the BOTTOM view's HUD is squeezed toward centre-right (radar ~x1280, sprint icon mid, weapon panel ~x1180) from level load, top view normal - separate bug, possibly same family as salty's 21:9 report. Posted to Discord. Box harness: `vpad.py 3` leaves slot 1 for a real pad (user's controller = player 1). Also open: salty's Reach side-by-side HUD asymmetric at 21:9 (3440x1440, 2P L/R, crosshair Centered) - margins differ left vs right in each half; SR388: H2 4P Save & Quit hang (asked if the co-op mod is on) and an "exiting level crash" (details asked); H2A 4P performance (~30 fps on the box); CE Back toggle mid-mission in 3-4P Anniversary untested; H2 intermittent load crash (2 seen, during probe builds).
000. **(2026-09-26) v1.8.0-experimental released** (commit c778e71, https://github.com/kirklandsig/AlphaRing/releases/tag/v1.8.0-experimental; announced on the AlphaRing Discord #general). Next (v1.9, user: "I don't care what it takes"): Halo CE Anniversary graphics with 4 players, and the Halo 4 HUD with black bars removed (see the 2026-09-26 entry's "Not done" notes).
000. **(2026-09-25) v1.7.0-experimental released** (user-approved, Halo 4 labelled work in progress): https://github.com/kirklandsig/AlphaRing/releases/tag/v1.7.0-experimental. Announced (user-approved) on megabitt01/AlphaRing PR #20 (to XiaoDanny, with the H3/ODST/H4/H2 findings incl. the H3 off-axis reticle lead for his open Reach question) and issue #25. Open: Halo 4 HUD canvas (and H4 3P untested), H2/CE stock divider line, CE 3P layout, in-game dual-wield check, second Codex pass (Codex login returned 401).
00. **(2026-09-25) v1.6.0-experimental released** (user-approved): https://github.com/kirklandsig/AlphaRing/releases/tag/v1.6.0-experimental (prerelease, DLL + screenshots). XiaoDanny thanked on megabitt01/AlphaRing#20 (merged) with a heads-up on the CPatch default-on/saved-off bug and the unsynchronized `g_writes` in his tree. Next: watch for tester reports - CE level end (freeze fix), real ultrawide/multi-monitor, 3P menus, ODST/Reach HUD sides for health/grenades/equipment.
00b. **Ideas not done:** "teleport to player 1" spawn-menu action for P3/P4 stuck outside the map (needs object_set_position per game); MegaBit-style per-game menu page lists; Halo 4 HUD/spawn; recolour for CE/H2 (they don't pass colours through the host).
0. **(2026-09-24) v1.5.0-experimental shipped** (4-player fixes + spawn menus). Announced (user-approved) in megabitt01/AlphaRing issue #25 (mentions WinterSquire + Priception); unload-hook fix sent upstream as PR #24 against `master-chief` (compiles there; not runtime-tested on his tree - no local vcpkg/SDL2). Next: watch #24/#25 for replies; hear back from real-pad play; check the 3-player menu layout in game; investigate the H2 co-op-mod quit hang if it bothers them
0b. **Spawn follow-ups (ideas):** spawned allies following the player (squad order/"follow" in H2 orders, H3 objectives), a "delete last spawn" action, Reach/H4 backends, prefer already-used squads for the hijack if a way to tell them apart is found (ODST/H2 runtime squad records are all-zero both for never-placed and wiped-out squads)
1. **Tag and release v1.4.5-experimental** (2026-07-17 fixes) and get the crashing user to retest — the ServiceTag `%ls` overread is the best root-cause candidate yet
4. ~~Consider PR to upstream~~ **DONE:** ServiceTag fix submitted as https://github.com/megabitt01/AlphaRing/pull/6 — follow up on review feedback
5. **Decide on MegaBit's Xbox dashboard UI** — adopt, ignore, or wait for it to stabilize (adds SDL2/vcpkg deps, replaces JSON settings)
6. **If Reach armor staleness or CE/H2 wrong-colors get reported** — adapt MegaBit's `get_player_profile` Reach sync / color-index mapping (see 2026-07-17 session notes for why blind porting is wrong)
7. **Investigate controller connection timing** - Controllers need to be connected before launch (ConnectedPadMask hot-plug refresh may already improve this — retest)
8. **Merge to stable** once testing confirms compatibility
9. **UI improvements** (future):
   - Clarify "Bind Controller" vs player controller assignment
   - Consider reorganizing Save Profile / Save Preset buttons

---

## Contact

- **This Fork**: kirklandsig (https://github.com/kirklandsig/AlphaRing)
- **Upstream**: thejackbitt (https://github.com/thejackbitt/AlphaRing)
- **Original**: WinterSquire (https://github.com/WinterSquire/AlphaRing)
