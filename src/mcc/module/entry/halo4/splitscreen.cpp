// Left/Right split screen for Halo 4 (mcc/splitscreen/LeftRight), after XiaoDanny's Reach port.
#include "halo4.h"

#include "mcc/module/entry/PreservingThunk.h"
#include "mcc/splitscreen/LeftRight.h"

#define Halo4SplitscreenEntry(name, offset, returnType, pDetour, ...) \
    returnType pDetour(__VA_ARGS__); \
    typedef returnType (*pDetour##_t)(__VA_ARGS__); \
    ::Entry name(Halo4SplitscreenEntrySet(), offset, pDetour); \
    returnType pDetour(__VA_ARGS__)

namespace Halo4::Entry::Splitscreen {
    namespace LeftRight = MCC::Splitscreen::LeftRight;

    // The bar painter sets the render state up itself before drawing (as Reach's does, blackbars.cpp).
    void BeforePainting(__int64 module) {
        ((void (*)(int, int))(module + OFFSET_HALO4_PF_RENDER_SETUP_1))(0, 1);
        ((void (*)(int))(module + OFFSET_HALO4_PF_RENDER_SETUP_2))(0);
    }

    // Each view's own title-safe box (LeftRight::ViewSafeBox, ViewportRect); a leaf, hence the preserving thunk.
    PreservedEntry(entry_viewport_rect, Halo4SplitscreenEntrySet(), OFFSET_HALO4_PF_COMPUTE_VIEWPORT_RECT,
                   viewport_rect, void* slot, void* players, void* view, void* box, __int64) {
        return LeftRight::ViewportRect(entry_viewport_rect.m_pOriginal, slot, players, view, box);
    }

    constexpr LeftRight::Gen3 kGame {
        OFFSET_HALO4_PV_SPLITSCREEN_TABLE, OFFSET_HALO4_PV_SCREEN_SIZE, OFFSET_HALO4_PF_SPLITSCREEN_PLAYER_COUNT,
        OFFSET_HALO4_PF_FILL_RECT, OFFSET_HALO4_PF_RT_POOL_RELEASE, OFFSET_HALO4_PF_RT_POOL_INIT,
        BeforePainting,
    };
    LeftRight::State s_state;

    // Halo 4 lays each view's HUD out from the view's table variant, the full-height halves' (3) taking the
    // two-player layout, which is as wide as a top/bottom half: in a Left/Right half it ran off the right edge.
    // The quarter layout is exactly a half's width.
    Halo4SplitscreenEntry(entry_hud_layout, OFFSET_HALO4_PF_HUD_LAYOUT, int, hud_layout, int user) {
        constexpr int kHalf = 0x80077, kQuarter = 0x80078;
        int layout = ((hud_layout_t)entry_hud_layout.m_pOriginal)(user);
        if (layout != kHalf || !LeftRight::Chosen()) return layout;
        __int64 module = entry_hud_layout.m_target - entry_hud_layout.m_offset;
        int players = ((int (*)())(module + OFFSET_HALO4_PF_SPLITSCREEN_PLAYER_COUNT))();
        return LeftRight::Active(players) ? kQuarter : layout;
    }

    Halo4SplitscreenEntry(entry_render, OFFSET_HALO4_PF_RENDER, void, render) {
        LeftRight::Frame(kGame, entry_render.m_target - entry_render.m_offset, s_state);
        ((render_t)entry_render.m_pOriginal)();
    }

    Halo4SplitscreenEntry(entry_bars, OFFSET_HALO4_PF_DRAW_SPLITSCREEN_BARS, void, draw_bars) {
        if (!LeftRight::PaintDividers(kGame, entry_bars.m_target - entry_bars.m_offset))
            ((draw_bars_t)entry_bars.m_pOriginal)();
    }

    // Halo 4 sizes a pool target's view variant in a helper of its own: the descriptor's size, then for
    // variant 3 three quarters of the width by half the height, both pairs alike. For Left/Right's half
    // surface the helper is asked for the unshrunk size (variant 0 of the same target) and that is halved
    // in width, the way the pool halves for its other variants.
    Halo4SplitscreenEntry(entry_rt_size, OFFSET_HALO4_PF_RT_VARIANT_SIZE, void, variant_size,
                          const unsigned char* desc, int* w, int* h, int* w2, int* h2, int* face, int index) {
        ((variant_size_t)entry_rt_size.m_pOriginal)(desc, w, h, w2, h2, face, index);
        if (!LeftRight::ResizesHalfSurface(s_state, index) || !(*(const unsigned*)(desc + 0x10) & 0x20)) return;

        int full_w, full_h, full_w2, full_h2, full_face = 0;
        ((variant_size_t)entry_rt_size.m_pOriginal)(desc, &full_w, &full_h, &full_w2, &full_h2, &full_face,
                                                  index & ~3);
        *w = *w2 = full_w / 2;
        *h = *h2 = full_h;
    }
}
