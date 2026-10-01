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
        // MCC may not have loaded XInput yet when we start; load it ourselves then.
        if ((hModule = GetModuleHandleA("XINPUT1_3.dll")) ||
            (hModule = GetModuleHandleA("XINPUT1_4.dll")) ||
            (hModule = GetModuleHandleA("XINPUT9_1_0.dll")) ||
            (hModule = LoadLibraryA("XINPUT1_4.dll")) ||
            (hModule = LoadLibraryA("XINPUT1_3.dll"))) {
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
    // stall for milliseconds, so GetXInputGetState refuses to poll empty slots.
    // Empty slots are re-probed one per 500ms (round-robin) so a single frame
    // never eats more than one slow probe; WM_DEVICECHANGE (see Window.cpp)
    // triggers an immediate full rescan so hot-plug is picked up at once, and
    // a failed poll of a connected slot drops it from the mask on the spot.
    // The probes call g_pXInputGetState, the hook's trampoline (the real
    // function, not XInputGetStateDetour), as the wrapper consults this mask.
    // Present (Input::Update) and the game's input (get_key_state) both poll
    // pads, so the cache below is kept under one lock - held only to read and
    // update it, never across a probe (an empty slot's can take milliseconds).
    // g_seen counts each slot's successful reads (probes and polls): a failed
    // poll that started before a newer success doesn't drop the slot.
    static SRWLOCK g_pad_lock = SRWLOCK_INIT;
    static DWORD g_connected_mask = 0;
    static ULONGLONG g_last_probe = 0;
    static DWORD g_next_probe_slot = 0;
    static DWORD g_seen[4] = {};
    static bool g_probing = false; // one probe batch at a time; callers meanwhile use the cached mask
    static volatile LONG g_rescan_requested = 1;  // full sweep on first use

    void RequestPadRescan() {
        InterlockedExchange(&g_rescan_requested, 1);
    }

    static DWORD ConnectedPadMask() {
        if (!g_pXInputGetState) return 0;

        DWORD probe = 0; // the slots to probe: all four on a rescan, else one empty slot every 500 ms
        AcquireSRWLockExclusive(&g_pad_lock);
        auto now = GetTickCount64();
        if (g_probing) {
            // a rescan asked for now stays pending for the next caller
        } else if (InterlockedExchange(&g_rescan_requested, 0)) {
            probe = 0xF;
            g_last_probe = now;
        } else if (now - g_last_probe >= 500) {
            g_last_probe = now;
            for (DWORD n = 0; n < 4; ++n) {
                DWORD i = (g_next_probe_slot + n) % 4;
                if (g_connected_mask & (1u << i))
                    continue;
                probe = 1u << i;
                g_next_probe_slot = (i + 1) % 4;
                break;
            }
        }
        g_probing = g_probing || probe != 0;
        DWORD mask = g_connected_mask;
        ReleaseSRWLockExclusive(&g_pad_lock);
        if (!probe) return mask;

        DWORD found = 0;
        for (DWORD i = 0; i < 4; ++i) {
            XINPUT_STATE state;
            if ((probe & (1u << i)) && g_pXInputGetState(i, &state) == ERROR_SUCCESS)
                found |= 1u << i;
        }
        AcquireSRWLockExclusive(&g_pad_lock);
        if (probe == 0xF) g_connected_mask = found; // a rescan has the last word
        else g_connected_mask |= found;              // a probe only adds a pad it found
        for (DWORD i = 0; i < 4; ++i)
            if (found & (1u << i)) ++g_seen[i];
        g_probing = false;
        mask = g_connected_mask;
        ReleaseSRWLockExclusive(&g_pad_lock);
        return mask;
    }

    bool GetXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
        if (!g_pXInputGetState || !pState) return false;
        memset(pState, 0, sizeof(XINPUT_STATE));
        if (dwUserIndex >= 4 || !(ConnectedPadMask() & (1u << dwUserIndex)))
            return false;
        AcquireSRWLockShared(&g_pad_lock);
        DWORD seen = g_seen[dwUserIndex];
        ReleaseSRWLockShared(&g_pad_lock);
        if (g_pXInputGetState(dwUserIndex, pState) != ERROR_SUCCESS) {
            AcquireSRWLockExclusive(&g_pad_lock);
            if (g_seen[dwUserIndex] == seen) // no newer success while this poll ran
                g_connected_mask &= ~(1u << dwUserIndex);
            ReleaseSRWLockExclusive(&g_pad_lock);
            memset(pState, 0, sizeof(XINPUT_STATE));
            return false;
        }
        AcquireSRWLockExclusive(&g_pad_lock);
        ++g_seen[dwUserIndex];
        ReleaseSRWLockExclusive(&g_pad_lock);
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