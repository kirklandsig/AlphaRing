#include "Input.h"

#include "common.h"

#include "imgui.h"
#include "global/Global.h"
#include "MenuConfig.h"

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

        g_menuConfig = MenuConfig::load();

        return true;
    }

    bool Shutdown() {
        return true;
    }

    // XInputGetState on a disconnected slot triggers device enumeration and can
    // stall for milliseconds, so the wrapper below refuses to poll empty slots.
    // Refreshing the mask on a slow cadence keeps hot-plug working; the probe
    // uses the raw pointer because the wrapper consults this mask.
    static DWORD ConnectedPadMask() {
        static DWORD mask = 0;
        static ULONGLONG last_check = 0;

        if (!g_pXInputGetState) return 0;

        auto now = GetTickCount64();
        if (last_check == 0 || now - last_check >= 500) {
            last_check = now;
            mask = 0;
            for (DWORD i = 0; i < 4; ++i) {
                XINPUT_STATE state;
                if (g_pXInputGetState(i, &state) == ERROR_SUCCESS)
                    mask |= 1u << i;
            }
        }
        return mask;
    }

    bool GetXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
        if (!g_pXInputGetState || !pState) return false;
        memset(pState, 0, sizeof(XINPUT_STATE));
        if (dwUserIndex >= 4 || !(ConnectedPadMask() & (1u << dwUserIndex)))
            return false;
        return g_pXInputGetState(dwUserIndex, pState) == ERROR_SUCCESS;
    }

    void SetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) {
        if (!g_pXInputSetState) return;
        g_pXInputSetState(dwUserIndex, pVibration);
    }

    bool Update() {
        static bool b_toggled = false;
        static bool b_pressed = false;
        XINPUT_STATE state;

        if (!GetXInputGetState(0, &state))
            return false;

        if ((state.Gamepad.wButtons & g_menuConfig.controllerComboMask) == g_menuConfig.controllerComboMask) {
            if (!b_toggled) {
                AlphaRing::Global::Global()->show_imgui = !AlphaRing::Global::Global()->show_imgui;
                b_toggled = true;
                return false;
            }
        } else {
            b_toggled = false;
        }

        if (AlphaRing::Global::Global()->show_imgui) {
            const auto f_speed = [](SHORT x, SHORT y) -> ImVec2 {
                // Mouse Move Speed for Gamepad
                const auto speed = 5.0f;
                // Normalize Move Speed
                const auto f_normalize = [](SHORT sThumb) -> float {
                    const auto deadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
                    return (abs(sThumb) > deadZone) ? (sThumb / 32767.0f) : 0.0f;
                };
                // Get Final Move Speed
                return {f_normalize(x) * speed, -f_normalize(y) * speed};
            };

            ImGuiIO& io = ImGui::GetIO();

            if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
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