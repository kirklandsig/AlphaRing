#pragma once

// Per-player HUD customization: move, scale or hide HUD elements and recolour the HUD, separately
// for each local player.
//
// Every MCC game asks MCC's host (CGameManager) for a per-element override right before placing
// each HUD element (see CGameManagerHud.cpp); Halo 3, ODST and Reach also pass every HUD colour,
// with the drawing player, through the host. Answering those calls for the player being drawn is
// the whole trick: the games apply the override to that element and put it back after drawing.
// Halo CE applies host offsets wrongly (both to X), so it gets its offsets from a placement hook
// instead (module/entry/halo1/hud.cpp).
namespace MCC::Hud {
    enum Element : int { MotionSensor, Shield, Weapon, Grenades, Crosshair, Equipment, Messages, kElementCount };

    struct ElementLayout {
        float x = 0.0f, y = 0.0f; // offset, as a fraction of the HUD canvas width/height
        float scale = 1.0f;
        bool hidden = false;
    };

    // Screen shapes (width / height) a player's HUD can be laid out in, centered in their view:
    // the game's own layout, the view's full width (its edges), or a box of that shape.
    constexpr float kFullWidth = 100.0f;
    constexpr float kAreas[] = {0.0f, kFullWidth, 21.0f / 9.0f, 16.0f / 9.0f, 4.0f / 3.0f};
    constexpr int kAreaCount = sizeof(kAreas) / sizeof(*kAreas);
    extern const char* const kAreaNames[kAreaCount];
    // The colour a recolour toward `hue` shows as in the menus (ImGui's packed colour).
    unsigned HueColor(float hue);

    struct PlayerHud {
        float scale = 1.0f;      // every element
        int area = 0;            // index in kAreas
        ElementLayout elements[kElementCount];
        bool recolor = false;
        float hue = 200.0f;      // degrees
        float strength = 1.0f;   // 0 = game colours, 1 = fully the chosen hue
    };

    constexpr int kPlayers = 4;

    // The settings, read by the games' HUD drawing; edited by the overlay window and the
    // players' controller menus. Load() runs at startup.
    PlayerHud& Player(int player);
    void Load();
    void Save();

    // Host callbacks (CGameManagerHud.cpp): true when `element` of the player whose HUD is being
    // drawn has an override, composed onto MCC's own values.
    bool Transform(int game_element, float* dx, float* dy, float* scale);
    bool Anchor(int game_element, int* anchor);
    unsigned Color(int user, unsigned argb);

    // Whether the running game's HUD can be customized.
    bool Supported();

    // Halo CE: pixels to move `element` of `player`'s HUD within a view of the given size.
    bool OffsetCE(int player, int game_element, float view_width, float view_height, float* dx, float* dy);

    // Overlay window.
    void ImGuiContext();
}
