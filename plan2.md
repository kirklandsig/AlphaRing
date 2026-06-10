Plan: Port Xbox Menu to ImGui Integration
Context
The project currently has two separate UI systems:

DLL ImGui overlay (main tool): DirectX 11-based, used for splitscreen, gamepad mapping, and game-specific UI
Xbox menu system (src/mcc/xbox/): SDL2-based, a sophisticated state machine with smooth animations, embedded audio feedback, and a hierarchical menu structure
The goal is to integrate the xbox menu's architectural patterns (StateMachine, Menu hierarchy, animation system) into the DLL's ImGui rendering pipeline, eliminating the SDL2 dependency and consolidating into a single rendering stack.

High-Level Approach
Port the xbox StateMachine to ImGui by creating a new CXboxContext class that:

Implements the ICContext interface for pluggable integration
Reuses the Menu/Option/Page data structures and StateMachine logic from src/mcc/xbox/
Replaces SDL2 rendering with ImGui rendering (no visual changes, just different backend)
Replaces SDL Audio with Windows audio or mute for now
Integrates with existing XInput handling via Input.cpp
Maintains animation timing and state transitions unchanged
Implementation Steps
Phase 1: Create ImGui-based Rendering Layer for Xbox Menu
File: src/render/imgui/game/xbox/CXboxContext.h (new)

#pragma once

#include "render/imgui/ICContext.h"

class CXboxContext : public ICContext {
public:
void render() override;

static CXboxContext instance;
};

extern ICContext* g_pXboxContext;
File: src/render/imgui/game/xbox/CXboxContext.cpp (new)

High-level rendering function that:

Calls stateMachine->update(dt) to advance animation state
Uses ImGui primitives to render the menu based on stateMachine->getState()
Adapts the rendering logic from render.hpp to ImGui calls
Key mappings:

SDL_Rect → ImVec2 positioning with ImGui::SetCursorPos()
drawButton() → ImGui::Button() or custom draw lists
drawText() → ImGui::Text() or text-on-drawlist
drawGradientRect() → ImDrawList::AddRectFilledMultiColor()
drawDot() → ImDrawList::AddCircleFilled()
Opacity blending → ImGui color with alpha adjustment
Phase 2: Create StateMachine Input Adapter
File: src/render/imgui/game/xbox/XboxInputAdapter.h (new)

#pragma once

#include <functional>

class XboxInputAdapter {
public:
// Called each frame from CXboxContext::render()
void update();

// Maps ImGui/XInput state to StateMachine handler calls
void processInput(StateMachine* stateMachine);
};
File: src/render/imgui/game/xbox/XboxInputAdapter.cpp (new)

Implementation:

Reads controller state from the same XInput wrapper used by Input.cpp
D-Pad → page/option navigation (Left/Right/Up/Down)
A button → option confirmation
B/Back button → menu close
Debounce logic to prevent multiple inputs per frame
Only process input when menu phase allows it (check currentState.phase)
Phase 3: Move and Adapt Xbox Menu Data Structures
File: src/render/imgui/game/xbox/XboxMenuStructures.h (new)

Copy from src/mcc/xbox/:

Menu class with pages array
Page struct
Option class with OptionType enum
MenuState struct
Phase enum
State struct (but remove SDL_Renderer, TTF_Font, Mix_Chunk dependencies)
File: src/render/imgui/game/xbox/StateMachine.h (new)

Copy from src/mcc/xbox/state.h, removing audio parameter:

class StateMachine {
public:
StateMachine(const Menu& menu); // No sounds array

// ... same handler methods ...
void update(float dt);
void render(ImDrawList* drawList, int windowWidth, int windowHeight);

bool isRunning() const;
State& getState();
};
File: src/render/imgui/game/xbox/StateMachine.cpp (new)

Copy and adapt from src/mcc/xbox/state.cpp:

Keep all transition logic unchanged
Remove Mix_PlayChannel() calls (or replace with no-op stubs for now)
Keep input handlers (they just update state, don't play sounds)
Phase 4: Create ImGui-based Rendering Implementation
File: src/render/imgui/game/xbox/XboxMenuRenderer.h (new)

#pragma once

#include <imgui.h>
#include "XboxMenuStructures.h"

class XboxMenuRenderer {
public:
// High-level render entry point
static void render(State& state, ImDrawList* drawList, int windowWidth, int windowHeight);

private:
// Phase-based render logic (from render.hpp)
static void renderMenuContent(State& state, ImDrawList* drawList,
float scale, ImVec2 pos, float opacity);
static void renderOption(State& state, ImDrawList* drawList,
const Option& option, ImVec2 pos, bool selected);
static void renderSubpage(State& state, ImDrawList* drawList, ImVec2 pos);
};
File: src/render/imgui/game/xbox/XboxMenuRenderer.cpp (new)

Implement rendering functions:

Calculate phase-based scale/position/opacity using same logic as render.hpp
Use ImDrawList for primitive drawing (not ImGui immediate API, for pixel-perfect control)
Render pages with animation offsets
Render options with selection state coloring
Render subpage overlay when in InIdle phase
Phase 5: Integrate CXboxContext into ImGui Pipeline
File: src/render/imgui/ImGui.cpp (modify)

Add include:
#include "./game/xbox/CXboxContext.h"
Add to pages array (assuming game ID 5 is available):
static ICContext* pages[7] {
nullptr, // Index 0
nullptr, // Index 1
g_pHalo3Context, // Index 2 (Halo 3)
nullptr, // Index 3
nullptr, // Index 4
g_pXboxContext, // Index 5 (Xbox Menu) ← NEW
nullptr, // Index 6
};
Bounds check still applies automatically (no changes needed)
Phase 6: Handle State Serialization
File: src/render/imgui/game/xbox/XboxMenuState.h (new)

Copy MenuState struct and save/load functions from src/mcc/xbox/state.cpp:

bool saveMenuStateBin(const MenuState& state, const std::string& path);
bool loadMenuStateBin(MenuState& state, const std::string& path);
File: src/render/imgui/game/xbox/XboxMenuState.cpp (new)

Use same binary format for compatibility with existing saves
Save to: ./alpha_ring/xbox_menu_state.bin (new location in AlphaRing config dir)
Critical Files to Modify
File	Changes	Impact
src/render/imgui/ImGui.cpp	Add CXboxContext to pages array + include	Low - single registration
src/render/imgui/ImGui.h	No changes needed	None
CMakeLists.txt	Add new files to xbox_imgui source list	Build only
Critical Files to Create (New)
File	Purpose
src/render/imgui/game/xbox/CXboxContext.h/.cpp	ICContext implementation
src/render/imgui/game/xbox/XboxMenuStructures.h	Menu/Option/Page/State structs
src/render/imgui/game/xbox/StateMachine.h/.cpp	State machine core logic
src/render/imgui/game/xbox/XboxInputAdapter.h/.cpp	Input → StateMachine bridge
src/render/imgui/game/xbox/XboxMenuRenderer.h/.cpp	ImGui rendering logic
src/render/imgui/game/xbox/XboxMenuState.h/.cpp	Serialization
Architectural Decisions
1. Reuse Data Structures, Not Code Duplication
Copy Menu/Option/Page/State structs into new location (isolated from SDL2)
Reduces coupling between old SDL2 build and new ImGui integration
Original src/mcc/xbox/ directory remains untouched (can be deleted later or kept as reference)
2. Use ImDrawList for Rendering
ImGui's ImDrawList provides low-level drawing primitives (rects, circles, text)
Allows pixel-perfect control matching the original SDL2 rendering
More direct than using ImGui immediate-mode API
3. No Audio for Now
Audio feedback is desirable but not critical for initial integration
Can add Windows audio (e.g., PlaySound) later if needed
Stub out audio calls for now (silent operation)
4. Controller Input via Existing XInput
Reuse Input.cpp's XInput wrapper instead of SDL GameController
Maintains consistency with rest of DLL
XInputAdapter detects button states from XINPUT_STATE
5. State Machine Unchanged
All animation logic, phase transitions, and timing remain identical
Only change: rendering backend (SDL2 → ImGui) and input source (SDL events → XInput)
Preserves the sophisticated state machine design
Testing Strategy
Build verification: Ensure new files compile and link
Visual testing: Run DLL, verify xbox menu renders correctly when game ID is set to 5
Input testing: Test D-Pad navigation, A/B buttons, page transitions
Animation testing: Verify phase transitions and scaling animations match original
State persistence: Save and load menu state, verify saved file exists
Integration testing: Verify menu doesn't interfere with other ImGui contexts
Dependencies
No new external dependencies - uses only:

ImGui (already in project)
XInput (already in project)
Standard C++ STL
Removes: SDL2, SDL_ttf, SDL_mixer (for xbox module only)
Rollback Plan
If integration fails:

Original src/mcc/xbox/ directory is untouched
New files are isolated in src/render/imgui/game/xbox/
Delete new directory, remove ImGui.cpp change, rebuild
Original functionality restored
Success Criteria
 CXboxContext renders without SDL2 dependencies
 Menu navigation works with D-Pad/A/B inputs
 Page transitions animate smoothly (0.25s phase durations)
 State persists across sessions (save/load)
 No interference with existing MCC/Halo3 contexts
 Input debouncing prevents duplicate presses