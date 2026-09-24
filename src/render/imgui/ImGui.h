#pragma once

struct ImFont;

namespace AlphaRing::Render::ImGui {
    bool Initialize();

    void Render();

    // Large bold font for the in-game controller menus.
    ImFont* MenuFont();
}
