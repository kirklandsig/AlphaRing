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
`build/Release/WTSAPI32.dll`

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
| `src/mcc/module/entry/entry.cpp`, `CModule.cpp` | Hook install/removal per game DLL (removed on unload) |
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
1b. **CE/H2A with 3-4 players always run Classic graphics** (Anniversary renderer only draws 2 views); CE/H2 need the Good Luck Cairo Workshop co-op fix mods for proper P3/P4 spawns
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

### Sensitivity Options Don't Work (Reported)
- User reports vertical/horizontal look sensitivity options don't do anything
- **Root cause identified:** `VerticalLookSensitivity` and `HorizontalLookSensitivity` in `CUserProfile.h` are **booleans**, not floats!
  - They're toggles (probably invert Y/X axis), not sensitivity values
  - The UI correctly shows checkboxes, but the labels are misleading
- **Actual sensitivity values** that ARE floats:
  - `ZoomLookSensitivityMultiplier`
  - `VehicleLookSensitivityMultiplier`
  - `MouseSensitivity`
- **No controller base sensitivity** exposed in the profile struct - may be stored elsewhere in game memory
- **Possible fix:** Rename UI labels to clarify these are invert toggles, not sensitivity values

---

## Session History

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

000. **(2026-09-25) v1.7.0-experimental released** (user-approved, Halo 4 labelled work in progress): https://github.com/kirklandsig/AlphaRing/releases/tag/v1.7.0-experimental. Open: Halo 4 HUD canvas (and H4 3P untested), H2/CE stock divider line, CE 3P layout, in-game dual-wield check, second Codex pass (Codex login returned 401).
00. **(2026-09-25) v1.6.0-experimental released** (user-approved): https://github.com/kirklandsig/AlphaRing/releases/tag/v1.6.0-experimental (prerelease, DLL + screenshots). XiaoDanny thanked on megabitt01/AlphaRing#20 (merged) with a heads-up on the CPatch default-on/saved-off bug and the unsynchronized `g_writes` in his tree. Next: watch for tester reports - CE level end (freeze fix), real ultrawide/multi-monitor, 3P menus, ODST/Reach HUD sides for health/grenades/equipment.
00b. **Ideas not done:** "teleport to player 1" spawn-menu action for P3/P4 stuck outside the map (needs object_set_position per game); MegaBit-style per-game menu page lists; Halo 4 HUD/spawn; recolour for CE/H2 (they don't pass colours through the host).
0. **(2026-09-24) v1.5.0-experimental shipped** (4-player fixes + spawn menus). Announced (user-approved) in megabitt01/AlphaRing issue #25 (mentions WinterSquire + Priception); unload-hook fix sent upstream as PR #24 against `master-chief` (compiles there; not runtime-tested on his tree - no local vcpkg/SDL2). Next: watch #24/#25 for replies; hear back from real-pad play; check the 3-player menu layout in game; investigate the H2 co-op-mod quit hang if it bothers them
0b. **Spawn follow-ups (ideas):** spawned allies following the player (squad order/"follow" in H2 orders, H3 objectives), a "delete last spawn" action, Reach/H4 backends, prefer already-used squads for the hijack if a way to tell them apart is found (ODST/H2 runtime squad records are all-zero both for never-placed and wiped-out squads)
1. **Tag and release v1.4.5-experimental** (2026-07-17 fixes) and get the crashing user to retest — the ServiceTag `%ls` overread is the best root-cause candidate yet
2. **Fix sensitivity UI labels** - Rename VerticalLookSensitivity/HorizontalLookSensitivity to clarify they're invert toggles
3. **Investigate if controller sensitivity exists** - May need to find where base look sensitivity is stored in game memory
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
