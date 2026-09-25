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
    // Mappings saved before the dual-wield and vehicle defaults existed have all of those actions unbound,
    // which left players 2-4 unable to use a left-hand weapon: binds them to their usual buttons. A mapping
    // with any of them bound is left as it is.
    void FillSharedActions();

    static const std::array<const char*, 17>* ButtonNames();
    static const std::array<const char*, 66>* ActionNames();
};