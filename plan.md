## Plan: Replace Dear ImGui with `src/mcc/xbox` UI (Option A)

TL;DR - Replace the existing Dear ImGui overlay by porting the SDL-based Xbox-style UI in `src/mcc/xbox` into the current Direct3D11 Present hook. Keep the xbox menu state, layout, animations, and sound assets; replace SDL rendering and SDL event handling with a D3D11 renderer and the existing game injection input path.

**Scope**
- Render the xbox menu using a new D3D11 renderer adapter.
- Maintain xbox layout, states, transitions, page navigation, and sound assets from `src/mcc/xbox`.
- Integrate into the current `IDXGISwapChain::Present()` hook in `src/render/d3d11/D3d11.cpp`.
- Replace the ImGui rendering path in `src/render/imgui/ImGui.cpp` with the new xbox UI path.
- Keep matrix/viewport compatibility with the game’s DX11 frame.
- Ensure controller and keyboard input is routed to the menu and hidden from the game while active.

**Excluded**
- Rewriting the xbox UI state machine or page definitions beyond minimal adaptation.
- Replacing the game engine’s native UI or attempting a deep engine hook beyond Present/ResizeBuffers.
- Porting SDL audio directly; instead, reuse data assets and play via a simple in-process audio playback mechanism if required.

---

## Phase 1: Investigation and design

1. Confirm the current ImGui render path and swapchain hook.
   - Review `src/render/d3d11/D3d11.cpp`: `Present` hook calls `ImGui::Render()` and initializes with `InitMainRender`.
   - Review `src/render/imgui/ImGui.cpp`: uses ImGui Win32 + DX11 backend, loads fonts, updates input, and renders draw data.
2. Confirm xbox UI architecture.
   - `src/mcc/xbox/main.cpp`: SDL main loop, event polling, sound initialization, render loop.
   - `src/mcc/xbox/render.hpp`: menu render logic using `SDL_Renderer*`, `TTF_Font*`, window size, and `State`.
   - `src/mcc/xbox/state.cpp` / `state.h`: state machine for menu phases and event handling.
   - `src/mcc/xbox/draw/*.hpp`: primitive draw methods for rects, buttons, icons, text.
3. Decide rendering strategy.
   - Use D3D11 render target created from the existing swapchain/backbuffer.
   - Render the xbox UI after the game frame but before Present completes.
   - Use DirectWrite/D2D1 only if needed for text rendering; otherwise use a texture-based font atlas.
4. Decide input mapping.
   - Use `AlphaRing::Input::Update()` or existing game input hooks to feed menu navigation.
   - Preserve current ImGui-style menu capture behavior via global flags in `src/global/Global.h`.

---

## Phase 2: Create the render adapter

### 2.1 Add the renderer abstraction

1. Create new files:
   - `src/render/xbox/XboxUIRenderer.h`
   - `src/render/xbox/XboxUIRenderer.cpp`
2. Define a minimal renderer API:
   - `bool Initialize(ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*)`
   - `void Shutdown()`
   - `void BeginFrame(int width, int height)`
   - `void DrawRect(float x, float y, float width, float height, Color color, float alpha)`
   - `void DrawOpaqueRect(...)`
   - `void DrawText(const char* text, float x, float y, float scale, Color color, float alpha)`
   - `void DrawIcon(const void* data, size_t len, float x, float y, float width, float height, float alpha)`
   - `void EndFrame()`
3. Implement the renderer internally using D3D11:
   - one dynamic vertex buffer and index buffer for quads.
   - one simple textured shader for text/icon quads.
   - one solid-color shader for rectangles and backgrounds.
   - device state setup only once; preserve/restoring game state as minimally as possible.
4. Add texture support:
   - create a D3D11 texture for icon data from `assets/mccIcon.h` and any embedded PNG assets.
   - optionally build a font texture atlas from a system font or use DirectWrite to rasterize glyphs into a D3D11 texture.
5. Ensure the renderer can draw all primitives used by `src/mcc/xbox/draw/*`.

### 2.2 Text rendering approach

Option A1 (preferred): DirectWrite + D2D1 shared texture
   - Render text into a DXGI-compatible texture once per changed string.
   - Map it to the D3D11 renderer as a shader resource.
   - Pros: crisp system font support, no custom font atlas.
   - Cons: adds dependency on `D2D1.lib` / `DWrite.lib` and integration complexity.

Option A2 (fallback): font atlas + textured quads
   - Build a texture atlas from a TTF font at runtime or pre-generate a palette.
   - Render text via `DrawText` using atlas glyphs.
   - Pros: keeps renderer self-contained.
   - Cons: more implementation effort and less flexible unicode support.

Recommendation: begin with DirectWrite/D2D1 if linking is easy; otherwise use a small pre-built bitmap font texture.

---

## Phase 3: Port the xbox draw path

### 3.1 Create a rendering bridge

1. Add new files:
   - `src/mcc/xbox/XboxUIBridge.h`
   - `src/mcc/xbox/XboxUIBridge.cpp`
2. Implement the bridge as a thin adapter:
   - convert calls from `renderMenu()` and `draw*()` functions from SDL primitives to `XboxUIRenderer` APIs.
   - map SDL colors to renderer color structs.
   - handle coordinate conversion and clipping if necessary.
3. Replace SDL-specific rendering calls in the xbox renderer files:
   - `drawRect()` -> `XboxUIRenderer::DrawRect`
   - `drawOpaqueRect()` -> `XboxUIRenderer::DrawOpaqueRect`
   - `drawText()` -> `XboxUIRenderer::DrawText`
   - `drawIcon()` -> `XboxUIRenderer::DrawIcon`
   - `drawButton()` -> combination of rect/text/icon draws.
4. Keep event/state logic intact in `state.cpp`; only swap out render target and drawing abstraction.

### 3.2 Remove SDL render dependency

1. Replace `SDL_Renderer*` parameters in `renderMenu()` and downstream functions with a generic renderer interface pointer or reference.
2. Remove SDL-specific includes from new renderer/bridge files.
3. Keep SDL audio asset headers if sound assets are still used; else move sound playback to a new audio helper.
4. Update `src/mcc/xbox/main.cpp` only as a reference/demo; the injected menu should not use the SDL window or SDL render loop.

---

## Phase 4: Integrate into the DX11 Present hook

### 4.1 Initialize the xbox renderer on D3D11 init

1. In `src/render/d3d11/D3d11.cpp::InitMainRender`, after `ImGui::Initialize()` currently runs, add `XboxUIRenderer::Initialize(Graphics()->pDevice, Graphics()->pContext, Graphics()->pSwapChain)`.
2. Add a new initialization control path that can choose between ImGui and xbox UI (debug toggle or compile-time define).
3. Ensure `Graphics()->RecreateRenderTargetView()` remains valid and that the xbox renderer uses the same RTV.

### 4.2 Render the xbox menu in Present

1. Replace `ImGui::Render()` in `Present` with a new function call `XboxUI::Render()`.
2. Create `src/render/xbox/XboxUI.cpp` and `XboxUI.h` with:
   - `bool Initialize()`
   - `void Shutdown()`
   - `void Render()`
   - `void Update()`
   - `void ToggleMenu()` or similar.
3. In `XboxUI::Render()`:
   - skip rendering when menu hidden.
   - call `XboxUIRenderer::BeginFrame()` with backbuffer dimensions.
   - invoke the xbox state/render bridge to draw the menu.
   - call `XboxUIRenderer::EndFrame()`.
4. If game frame clears the RTV after we render, ensure the xbox render call occurs after the game draw and before `Present` returns.

### 4.3 Manage menu visibility and menu capture

1. Reuse `AlphaRing::Global::Global()->show_imgui` as the global menu toggle or add `show_xbox_menu` alongside it.
2. Add a new bool in `src/global/Global.h` for the xbox menu if desired.
3. Mirror ImGui’s input suppression behavior:
   - when menu open, set `disable_input_on_menu_shown = true`.
   - if needed, use a dedicated menu flag to prevent game action input.
4. Set a runtime toggle to switch between ImGui and xbox UI for validation.

---

## Phase 5: Input handling

### 5.1 Reuse game input update

1. Keep `AlphaRing::Input::Update()` inside the menu render path, as the current ImGui path does.
2. Forward relevant input state into xbox menu event handlers.
3. Prefer using existing game input data rather than new SDL events.

### 5.2 Map input to xbox menu control

1. Add a mapping layer in `src/mcc/xbox/bridge` or `XboxUI.cpp`:
   - D-pad / arrow keys -> page/option navigation
   - A / Enter -> select/activate
   - B / Escape -> back/close
   - shoulder buttons -> page shift if used
2. If the xbox UI uses `SDL_GameController`, map equivalent XInput or Win32 controller state.
3. Use the existing xbox state machine’s event API to handle navigation transitions.

### 5.3 Preserve game input suppression

1. When the new menu is visible, ensure the game does not respond to the same input.
2. If the DLL already uses a hook or global flag for this, reuse it; otherwise add one in `global/Global.h` or input code.

---

## Phase 6: Audio and asset reuse

1. Keep the embedded WAV assets from `src/mcc/xbox/assets/*.h`.
2. Implement an in-process audio player if SDL audio is removed.
   - Option: use XAudio2 or `PlaySound` on memory buffer.
   - Option: keep SDL_mixer only if porting is too expensive, but avoid SDL window dependencies.
3. Load the embedded wave data into audio buffers and play successes/failures on menu open/close/navigation/select.
4. Preserve `StateMachine` sound triggers if they exist.

---

## Phase 7: CMake and build updates

1. Add new source files to `CMakeLists.txt` under `CORE_SRC` or explicitly list them.
2. Add required libraries if using DirectWrite/D2D1:
   - `D2D1.lib`
   - `Dwrite.lib`
3. Remove or isolate SDL dependencies from the core DLL build; do not link `SDL2` unless the injected menu still needs it.
4. Keep `src/mcc/xbox/main.cpp` as an optional demo binary only if needed; otherwise exclude it from the shared library build.
5. If you create a static renderer module, ensure its headers are included in `src/` and the import path is valid.

---

## Phase 8: Fallback and validation

1. Add a runtime debug flag or config option:
   - `use_xbox_ui` vs `use_imgui`
   - `show_xbox_ui`
2. Keep the ImGui code path intact in `src/render/imgui/ImGui.cpp` until the xbox UI is validated.
3. Provide a hotkey or compile-time switch to restore ImGui quickly.
4. Build verification:
   - compile the DLL in Release and Debug.
   - use `cmake --build . --config Release`.
5. Runtime verification:
   - inject the DLL into MCC.
   - open the menu, verify Xbox-style menu shows.
   - verify the menu renders correctly over the game frame.
   - verify keyboard/controller navigation and selection.
   - verify input suppression when the menu is open.
   - verify closing the menu restores game rendering.

---

## File-level change list

### New files
- `src/render/xbox/XboxUIRenderer.h` — D3D11 renderer interface and resources.
- `src/render/xbox/XboxUIRenderer.cpp` — implementation of quad batching, shaders, text/icon rendering.
- `src/mcc/xbox/XboxUIBridge.h` — abstraction from `src/mcc/xbox` draw APIs to D3D11 renderer.
- `src/mcc/xbox/XboxUIBridge.cpp` — implementation of the draw bridge and renderer adapters.
- `src/render/xbox/XboxUI.cpp` — menu render controller and integration wrapper.
- `src/render/xbox/XboxUI.h` — menu API used by D3D11 Present hook.
- `src/render/xbox/XboxUIText.cpp` / `XboxUIText.h` (optional) — if text rendering is separated.
- `src/render/xbox/XboxUIAudio.cpp` / `XboxUIAudio.h` (optional) — sound playback helper.

### Modified files
- `src/render/d3d11/D3d11.cpp` — hook initialization and `Present()` to call Xbox UI render path.
- `src/render/imgui/ImGui.cpp` — gate or disable current ImGui render path, preserve code for fallback.
- `src/global/Global.h` — add Xbox menu control flags if needed.
- `src/mcc/xbox/render.hpp` — replace SDL renderer API with generic renderer interface.
- `src/mcc/xbox/draw/*.hpp` — update draw primitive implementations to call generic renderer APIs instead of SDL.
- `CMakeLists.txt` — include new files and any new link dependencies.

---

## Detailed implementation checklist

### A. Renderer initialization
- [ ] Create the D3D11 renderer module.
- [ ] Initialize on first `Present()` via `InitMainRender()`.
- [ ] Reuse the same swapchain/device/context as the game.
- [ ] Create shaders for solid fills and textured quads.
- [ ] Create a font texture or DirectWrite bridge.
- [ ] Create icon texture(s) from embedded assets.

### B. UI rendering bridge
- [ ] Define a generic renderer interface for xbox menu draws.
- [ ] Port `renderMenu()` and draw helpers to use the renderer interface.
- [ ] Ensure all menu transitions and layout math remain unchanged.
- [ ] Preserve `StateMachine` update and event semantics.

### C. Present hook integration
- [ ] Add `XboxUI::Render()` call to `Present()` instead of `ImGui::Render()`.
- [ ] Add `XboxUI::Initialize()` during D3D11 init.
- [ ] Ensure `ResizeBuffers()` still recreates render target view correctly.
- [ ] Preserve game render state around menu draws.

### D. Input capture and event mapping
- [ ] Use existing input update path.
- [ ] Map input state to xbox menu navigation.
- [ ] Suppress game input when the menu is active.
- [ ] Add menu open/close toggle.

### E. Audio and asset handling
- [ ] Load embedded wav assets without SDL window.
- [ ] Play audio on menu open/close/navigation/select.
- [ ] Validate audio works in-process without SDL event looping.

### F. Build and fallback
- [ ] Update CMake to include new render module.
- [ ] Add DirectWrite/D2D1 libs if used.
- [ ] Keep ImGui code as a fallback path.
- [ ] Add runtime or compile-time UI selection.

---

## Verification plan

1. Build verification:
   - `cmake --build . --config Release`
   - ensure linker includes any new dependencies.
2. Runtime smoke test:
   - inject the DLL into MCC.
   - open the menu and verify it renders instead of ImGui.
3. Behavior verification:
   - navigate pages and options with both controller and keyboard.
   - verify animations, page transitions, and alpha blending.
   - verify menu open/close sounds play.
   - verify the game does not respond to menu input when open.
4. Regression check:
   - switch back to ImGui fallback and confirm original menu returns.
   - confirm `ResizeBuffers()` still works on window resize.
5. Performance check:
   - measure frame time with menu closed vs open.
   - target minimal overhead; ensure no frame drops from menu render.

---

## Risks and mitigations

- Risk: text rendering complexity. Mitigation: start with DirectWrite/D2D1 bridge or choose a small bitmap font atlas.
- Risk: preservation of game render state. Mitigation: save/restore only changed D3D11 state and test on the actual game.
- Risk: input conflict. Mitigation: use existing menu capture flags and route input through the existing input subsystem.
- Risk: asset loading or audio playback dependency creep. Mitigation: keep SDL usage limited to debug/demo or eliminate SDL entirely in the injected DLL.

---

## Recommended order of implementation

1. Build the renderer abstraction and confirm it compiles cleanly.
2. Port one simple draw primitive and validate rendering in `Present()`.
3. Port the full `renderMenu()` pipeline.
4. Add input mapping and menu toggle.
5. Add audio playback and validate choice.
6. Finalize build integration and cleanup ImGui fallback.

---

## Notes

- The existing xbox UI code is currently a standalone SDL app, not a D3D11 overlay.
- `src/render/d3d11/D3d11.cpp` is the correct integration point for the injected UI.
- Use the xbox state machine and layout directly to avoid re-implementing menu behavior.
- Keep the existing ImGui path around until the new UI is validated.

