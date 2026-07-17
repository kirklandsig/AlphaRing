#pragma once

#include <Windows.h>
#include <Xinput.h>

namespace AlphaRing {
    namespace Input {
        bool Init();
        bool Shutdown();
        bool Update();
        // Safe for per-frame polling: skips disconnected slots (500ms-cached
        // connected mask) to avoid XInputGetState's ms-scale stall on empty
        // slots, and always zeroes *pState when returning false.
        bool GetXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
        void SetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
    };
}