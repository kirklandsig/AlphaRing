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

### Branches

| Branch | Purpose |
|--------|---------|
| `dev` | Main development branch (shown on GitHub) |
| `feature/controller-bind-and-default-mappings` | All working features |
| `experimental/proton-compat` | Proton compatibility experiments (latest work) |

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

1. **Tag and release v1.4.5-experimental** (2026-07-17 fixes) and get the crashing user to retest — the ServiceTag `%ls` overread is the best root-cause candidate yet
2. **Fix sensitivity UI labels** - Rename VerticalLookSensitivity/HorizontalLookSensitivity to clarify they're invert toggles
3. **Investigate if controller sensitivity exists** - May need to find where base look sensitivity is stored in game memory
4. **Consider PR to upstream** (megabitt01/AlphaRing) with the ServiceTag fixes — he already ports our work, and his master shares the vulnerable merge-base code
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
