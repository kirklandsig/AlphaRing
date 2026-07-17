#pragma once

#include <Windows.h>
#include <Xinput.h>

namespace AlphaRing {
    namespace Input {
        bool Init();
        bool Shutdown();
        bool Update();
        // Safe for per-frame polling: skips disconnected slots (cached
        // connected mask) to avoid XInputGetState's ms-scale stall on empty
        // slots, and always zeroes *pState when returning false.
        bool GetXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
        // Ask for an immediate full rescan of controller slots on the next
        // poll; call on WM_DEVICECHANGE so hot-plug is detected instantly.
        void RequestPadRescan();
        void SetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
    };
}