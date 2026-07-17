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
    // Empty slots are re-probed one per 500ms (round-robin) so a single tick
    // never eats more than one slow probe; WM_DEVICECHANGE (see Window.cpp)
    // triggers an immediate full rescan so hot-plug is picked up instantly, and
    // a failed poll of a connected slot drops it from the mask on the spot.
    // The probes use the raw pointer because the wrapper consults this mask.
    static DWORD g_connected_mask = 0;
    static volatile LONG g_rescan_requested = 1;  // full sweep on first use

    void RequestPadRescan() {
        InterlockedExchange(&g_rescan_requested, 1);
    }

    static DWORD ConnectedPadMask() {
        static ULONGLONG last_probe = 0;
        static DWORD next_probe_slot = 0;

        if (!g_pXInputGetState) return 0;

        auto now = GetTickCount64();
        if (InterlockedExchange(&g_rescan_requested, 0)) {
            DWORD mask = 0;
            for (DWORD i = 0; i < 4; ++i) {
                XINPUT_STATE state;
                if (g_pXInputGetState(i, &state) == ERROR_SUCCESS)
                    mask |= 1u << i;
            }
            g_connected_mask = mask;
            last_probe = now;
        } else if (now - last_probe >= 500) {
            last_probe = now;
            for (DWORD n = 0; n < 4; ++n) {
                DWORD i = (next_probe_slot + n) % 4;
                if (g_connected_mask & (1u << i))
                    continue;
                XINPUT_STATE state;
                if (g_pXInputGetState(i, &state) == ERROR_SUCCESS)
                    g_connected_mask |= 1u << i;
                next_probe_slot = (i + 1) % 4;
                break;
            }
        }
        return g_connected_mask;
    }

    bool GetXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
        if (!g_pXInputGetState || !pState) return false;
        memset(pState, 0, sizeof(XINPUT_STATE));
        if (dwUserIndex >= 4 || !(ConnectedPadMask() & (1u << dwUserIndex)))
            return false;
        if (g_pXInputGetState(dwUserIndex, pState) != ERROR_SUCCESS) {
            g_connected_mask &= ~(1u << dwUserIndex);
            memset(pState, 0, sizeof(XINPUT_STATE));
            return false;
        }
        return true;
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