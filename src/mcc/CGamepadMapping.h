#pragma once

#include <array>

struct CGamepadMapping {
    enum eButton : __int8 {
        LeftTrigger, RightTrigger,
        DpadUp, DpadDown, DpadLeft, DpadRight,
        Start, Back,
        LeftThumb, RightThumb,
        LeftShoulder, RightShoulder,
        A, B, X, Y,
        None = -1  // Unbound - no button assigned
    };

    eButton actions[66];

    void ImGuiContext();
    void ResetToDefaults();

    static const std::array<const char*, 17>* ButtonNames();
    static const std::array<const char*, 66>* ActionNames();
};