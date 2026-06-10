Plan: Bridging CXboxMenu/CStateMachine into the Mod's Render Pipeline
Overview
The SDL2 main.cpp had three responsibilities: init (renderer + audio + controller), loop (poll events → update → render → present), and teardown. The mod already handles all three at the framework level via the D3D11 Present hook. The bridge is a new ICContext subclass — CXboxContext — that slots into the existing ImGui::Render() call and replaces the SDL2 application loop entirely.

Step 1 — Audio Subsystem Init
Problem: CStateMachine takes std::array<Mix_Chunk*, 6>, so SDL_mixer must be initialized before the context is created. The game's audio is DirectSound/XAudio2 so SDL_mixer won't conflict as long as it opens its own device.

Where: AlphaRing::Render::ImGui::Initialize() in src/render/imgui/ImGui.cpp.

Add after ImGui_ImplDX11_Init(...):


SDL_Init(SDL_INIT_AUDIO);
Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
And in a corresponding Shutdown() path, add Mix_CloseAudio() / SDL_QuitSubSystem(SDL_INIT_AUDIO).

Risk: If MCC or another injected DLL has already called SDL_Init, this is a no-op. If nothing has, this opens a new audio device cleanly.

Step 2 — Load the Xbox Menu Font
StateMachine::render() takes ImFont*. The mod currently loads Segoe UI equivalent (Chinese font). The Xbox menu needs its own Segoe UI font at ~25px to match the original.

Where: Same Initialize() call, after io.Fonts->Clear() and before ImGui_ImplDX11_Init.


// Load second font for Xbox overlay
ImFont* g_pXboxFont = io.Fonts->AddFontFromFileTTF(
    R"(C:\Windows\Fonts\segoeui.ttf)", 25.0f * scale);
Store this pointer where CXboxContext can access it (pass it to the constructor or expose it as a global in src/render/imgui/ImGui.h).

Step 3 — Create CXboxContext
New file: src/render/imgui/game/xbox/CXboxContext.h / .cpp


class CXboxContext : public ICContext {
    Menu           m_menu;
    StateMachine   m_stateMachine;     // owns the state + phase transitions
    bool           m_open = false;
    ImFont*        m_font;
    Uint32         m_lastTick = 0;

public:
    CXboxContext(ImFont* font);         // loads WAV chunks, constructs StateMachine
    ~CXboxContext();                   // frees Mix_Chunks, saves state

    void open();                       // triggers Opening phase + plays open sound
    void close();                      // triggers Closing phase via StateMachine
    bool isOpen() const;

    void handleInput(InputCommand cmd);// forwards to m_stateMachine.handleInput()
    void render() override;            // called every frame by ImGui::Render()
};
Constructor: Load the 6 WAV assets from the inline headers (already in src/render/imgui/game/xbox/assets/) via Mix_LoadWAV_RW(SDL_RWFromConstMem(...), 1). Pass the resulting array to StateMachine. Load saved state via loadMenuStateBin.

render():

If !m_open && !m_stateMachine.isRunning(), return early — nothing to draw.
Compute float dt from SDL_GetTicks() delta.
Call m_stateMachine.update(dt).
Get viewport size from ImGui::GetMainViewport().
Call m_stateMachine.render(width, height, m_font).
If m_open && !m_stateMachine.isRunning(), set m_open = false and save state — this is the natural close-phase-ended signal.
open(): Set m_open = true. The StateMachine constructor already initializes into the Opening phase, so for re-opens you'll need either a StateMachine::reset() method or recreate the instance.

Step 4 — Extend Input.cpp for Xbox Menu Navigation
Where: src/input/Input.cpp, inside Update().

The existing code maps right thumbstick → mouse movement. When the Xbox menu is open, D-pad and face buttons must instead drive InputCommand. Add edge-detection statics for each button to fire once per press, not once per frame:


// When g_pXboxContext->isOpen():
DPAD_UP    → InputCommand::Up
DPAD_DOWN  → InputCommand::Down
DPAD_LEFT  → InputCommand::Left
DPAD_RIGHT → InputCommand::Right
A          → InputCommand::Select
B          → InputCommand::Back
When the Xbox menu is open, suppress the thumbstick-to-mouse mapping so both modes don't fight.

g_pXboxContext needs to be accessible from Input.cpp — a forward declaration plus a getter in src/render/imgui/game/xbox/CXboxContext.h works.

Step 5 — Toggle Binding
The existing Start+Back combo toggles show_imgui. Use a distinct combo for the Xbox menu to avoid mode confusion. Recommended: Back alone (a single tap of Back while the debug UI is closed, distinct from the Back+Start chord).

In Input::Update(), before the thumbstick block:


// Back alone (not Start+Back) → toggle Xbox menu
if ((buttons & XINPUT_GAMEPAD_BACK) && !(buttons & XINPUT_GAMEPAD_START)) {
    if (!b_back_toggled) {
        if (g_pXboxContext->isOpen()) g_pXboxContext->close();
        else g_pXboxContext->open();
        b_back_toggled = true;
    }
} else { b_back_toggled = false; }
Step 6 — Register in ImGui::Render()
Where: src/render/imgui/ImGui.cpp.

Add #include "game/xbox/CXboxContext.h"
Declare CXboxContext* g_pXboxContext = nullptr;
In Initialize(), after audio init: g_pXboxContext = new CXboxContext(g_pXboxFont);
In Render(), call g_pXboxContext->render(); alongside g_pMCCContext->render(). It belongs here (not in the game-specific pages[]) since it's MCC-level, not game-specific.
Note: g_pXboxContext->render() must be called even when show_imgui is false — the Xbox overlay should be visible independently of the debug menus. This means its render call needs to sit inside the ImGui::NewFrame() / ImGui::Render() block unconditionally, using ImGui::SetNextWindowPos with no title bar to draw as a borderless fullscreen overlay.

Summary of New/Modified Files
File	Change
src/render/imgui/game/xbox/CXboxContext.h	New — ICContext subclass declaration
src/render/imgui/game/xbox/CXboxContext.cpp	New — owns StateMachine, audio, open/close
src/render/imgui/ImGui.cpp	Add audio init, font load, CXboxContext instantiation + render call
src/input/Input.cpp	Add D-pad/face button → InputCommand routing + toggle binding
src/render/imgui/game/xbox/CStateMachine.h	Minor: add reset() or open() method if re-open needed
The show_imgui flag and the Xbox menu open/close are independent — users can have the debug UI hidden while the Xbox overlay is open, and vice versa.