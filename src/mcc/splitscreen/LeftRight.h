#pragma once

#include "mcc/module/patch/SplitscreenConfigStore.h"

// The Left/Right split screen in every game. One saved choice (the store's TwoPlayerLayout) drives them all.
// XiaoDanny's Reach port (mcc/module/patch/SplitscreenConfigStore, module/entry/haloreach) showed what the
// layout takes: the split-screen table's regions, the black-bar painter, the render target the half-screen
// views draw into and the HUD's shape. Halo 3, ODST and Halo 4 share Reach's table and render-target pool
// design, so each says where those live in a Gen3 below; Halo 2 and CE lay their views out on a grid
// (module/entry/halo2, halo1).
namespace MCC::Splitscreen::LeftRight {
    // Left/Right is the saved choice.
    bool Chosen();

    // Left/Right is what `player_count` local players see: it is chosen and 2 or 3 players are on screen.
    bool Active(int player_count);

    // `game` (CGameGlobal::eGame) has the layout for `player_count` players (Halo CE: two only).
    bool Supports(int game, int player_count);

    // Left/Right is what `player_count` players of `game` see.
    bool OnScreen(int game, int player_count);

    // Halo CE and Halo 2 are side by side only in Classic graphics, which is set as a mission starts
    // (Splitscreen::ClassicGraphicsScope), so they take the choice then: whether the mission running now
    // is side by side, and whether the choice waits for the next mission start in `game`.
    void StartClassicMission(int game, bool side_by_side);
    bool WaitsForNextMission(int game, int player_count);

    // Saves the choice and puts it on screen in whichever game is running.
    void Choose(bool left_right);

    // Where a Gen3 engine keeps what the layout touches (module-relative addresses).
    struct Gen3 {
        __int64 table;          // c_splitscreen_config::m_config_table, {x0, y0, x1, y1, variant}[players * 4 + slot]
        __int64 screen;         // int width, height the views are laid out on
        __int64 player_count;   // int () - local players on screen
        __int64 fill_rect;      // void (short rect[4] {top, left, bottom, right}, unsigned argb)
        __int64 pool_release;   // the render-target pool teardown and rebuild the engine's resize path runs
        __int64 pool_init;
        void (*before_painting)(__int64 module) = nullptr; // render state the painter sets up itself (Halo 4)
        bool double_rounding = false; // render-target sizes are rounded in double precision (Halo 3), not float
        int hud_layout_size = 0;      // bytes in a HUD layout record (chud globals) ...
        int hud_canvas = 0;           // ... and where it keeps its int canvas width, height
    };

    constexpr int kMaxHudLayoutSize = 0x110;

    // Per-game runtime state, one per game module.
    struct State {
        __int64 module = 0;
        bool table_left_right = false;                     // the table holds the Left/Right regions
        AlphaRing::SplitscreenConfigStore::LayoutEntry saved[5]{}; // the entries they replaced (kSlots)
        unsigned built = 0;                                // layout generation the render targets were made for
        bool tall_hud[4]{};                                // the user's HUD was switched to a full-height half's
        unsigned char hud_layouts[4][kMaxHudLayoutSize]{}; // ... layout, and its layout record, reshaped
    };

    // Before each frame is drawn: keeps the table on the chosen layout (restoring the game's own regions,
    // black-bar patches included, when Left/Right is turned off) and rebuilds the render targets when the
    // choice changed since they were made.
    void Frame(const Gen3& game, __int64 module, State& state);

    // In place of the black-bar painter: draws the Left/Right dividers and returns true while the layout
    // is on screen; returns false to let the game paint its own bars.
    bool PaintDividers(const Gen3& game, __int64 module);

    // The HUD of a full-height half. The engine picks a player's HUD layout by the shape of their view
    // (resolution 1 for a two-player half, 4 for a quarter; 5 and 6 on 4:3 screens) and each layout has a
    // virtual canvas, stretched over the view. No layout is taller than 4:3, so a Left/Right half takes a
    // quarter's layout - made for a view half the screen wide - and its canvas is made as tall as the
    // view, which keeps the HUD at a quarter's size without stretching it and puts the elements the
    // layout anchors at the bottom (the motion tracker) at the bottom of the half.
    int HudResolution(const Gen3& game, __int64 module, State& state, int user, int resolution);
    const void* HudLayout(const Gen3& game, __int64 module, State& state, int user, const void* layout);

    // From a render-target sizing hook: records that the pool is being built for the current choice, and
    // returns true when a target of this variant is the full-height halves' surface while Left/Right is
    // chosen - the caller then makes it half the screen's width and its full height.
    bool ResizesHalfSurface(State& state, int variant);

    // ResizesHalfSurface for the pool's creation hook (Halo 3, ODST): `sizes` holds the target's two {w, h}
    // pairs (at + 0x18 and + 0x04), already shrunk to the stock variant's three quarters by half. The
    // unshrunk sizes are recomputed from the descriptor and only used when they reproduce the pool's own
    // result, so a misread descriptor keeps the stock size.
    void SizeSurface(const Gen3& game, __int64 module, State& state, int* sizes, const unsigned char* desc,
                     int variant);
}
