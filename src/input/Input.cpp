#include "Input.h"

#include "common.h"

#include "imgui.h"
#include "global/Global.h"
#include "render/imgui/game/xbox/CXboxContext.h"

static HMODULE hModule;
static DWORD (WINAPI* g_pXInputGetState)(_In_  DWORD dwUserIndex, _Out_ XINPUT_STATE* pState) WIN_NOEXCEPT;
static DWORD (WINAPI* g_pXInputSetState)(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration) WIN_NOEXCEPT;

namespace AlphaRing::Input {
    bool Init() {
        if ((hModule = GetModuleHandleA("XINPUT1_3.dll")) ||
            (hModule = GetModuleHandleA("XINPUT1_4.dll")) ||
            (hModule = GetModuleHandleA("XINPUT9_1_0.dll"))) {
            g_pXInputGetState = (decltype(g_pXInputGetState))GetProcAddress(hModule, "XInputGetState");
            g_pXInputSetState = (decltype(g_pXInputSetState))GetProcAddress(hModule, "XInputSetState");
        }

        assertm(hModule != nullptr, "failed to find xinput module");

        return true;
    }

    bool Shutdown() {
        return true;
    }

    bool GetXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
        if (!g_pXInputGetState || !pState) return false;
        memset(pState, 0, sizeof(XINPUT_STATE));
        return g_pXInputGetState(dwUserIndex, pState) == ERROR_SUCCESS;
    }

    void SetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) {
        if (!g_pXInputSetState) return;
        g_pXInputSetState(dwUserIndex, pVibration);
    }

    bool Update() {
        static bool b_toggled = false;
        static bool b_pressed = false;
        static WORD prevButtons = 0;
        XINPUT_STATE state;

        if (!GetXInputGetState(0, &state))
            return false;

        WORD buttons = state.Gamepad.wButtons;
        WORD justPressed = buttons & ~prevButtons;
        prevButtons = buttons;

        // Start+Back: toggle debug UI
        if (buttons & XINPUT_GAMEPAD_START && buttons & XINPUT_GAMEPAD_BACK) {
            if (!b_toggled) {
                AlphaRing::Global::Global()->show_imgui = !AlphaRing::Global::Global()->show_imgui;
                b_toggled = true;
                return false;
            }
        } else {
            b_toggled = false;
        }

        // Back alone: toggle Xbox overlay menu
        if ((justPressed & XINPUT_GAMEPAD_BACK) && !(buttons & XINPUT_GAMEPAD_START)) {
            if (g_pXboxContext) {
                if (g_pXboxContext->isOpen()) g_pXboxContext->close();
                else g_pXboxContext->open();
            }
        }

        // D-pad and face buttons drive Xbox menu navigation
        if (g_pXboxContext && g_pXboxContext->isOpen()) {
            const auto send = [&](WORD btn, InputCommand cmd) {
                if (justPressed & btn) g_pXboxContext->handleInput(cmd);
            };
            send(XINPUT_GAMEPAD_DPAD_UP,    InputCommand::Up);
            send(XINPUT_GAMEPAD_DPAD_DOWN,  InputCommand::Down);
            send(XINPUT_GAMEPAD_DPAD_LEFT,  InputCommand::Left);
            send(XINPUT_GAMEPAD_DPAD_RIGHT, InputCommand::Right);
            send(XINPUT_GAMEPAD_A,          InputCommand::Select);
            send(XINPUT_GAMEPAD_B,          InputCommand::Back);
        }

        // Thumbstick mouse control for debug UI (suppressed while Xbox menu is open)
        if (AlphaRing::Global::Global()->show_imgui && !(g_pXboxContext && g_pXboxContext->isOpen())) {
            const auto f_speed = [](SHORT x, SHORT y) -> ImVec2 {
                const auto speed = 5.0f;
                const auto f_normalize = [](SHORT sThumb) -> float {
                    const auto deadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
                    return (abs(sThumb) > deadZone) ? (sThumb / 32767.0f) : 0.0f;
                };
                return {f_normalize(x) * speed, -f_normalize(y) * speed};
            };

            ImGuiIO& io = ImGui::GetIO();

            if (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
                if (!b_pressed) {
                    io.MouseDown[0] = true;
                    b_pressed = true;
                }
            } else if (b_pressed) {
                io.MouseDown[0] = false;
                b_pressed = false;
            }

            io.MousePos += f_speed(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
        }

        return true;
    }
}