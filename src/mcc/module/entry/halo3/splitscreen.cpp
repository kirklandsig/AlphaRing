// Left/Right split screen for Halo 3 (mcc/splitscreen/LeftRight), after XiaoDanny's Reach port.
#include "halo3.h"

#include "mcc/splitscreen/LeftRight.h"

namespace Halo3::Entry::Splitscreen {
    namespace LeftRight = MCC::Splitscreen::LeftRight;

    constexpr LeftRight::Gen3 kGame {
        OFFSET_HALO3_PV_SPLITSCREEN_TABLE, OFFSET_HALO3_PV_SCREEN_SIZE, OFFSET_HALO3_PF_SPLITSCREEN_PLAYER_COUNT,
        OFFSET_HALO3_PF_FILL_RECT, OFFSET_HALO3_PF_RT_POOL_RELEASE, OFFSET_HALO3_PF_RT_POOL_INIT,
        nullptr, true, 0x64, 0x10,
    };
    LeftRight::State s_state;

    Halo3Entry(entry_render, OFFSET_HALO3_PF_RENDER, void, render) {
        LeftRight::Frame(kGame, entry_render.m_target - entry_render.m_offset, s_state);
        ((render_t)entry_render.m_pOriginal)();
    }

    Halo3Entry(entry_bars, OFFSET_HALO3_PF_DRAW_SPLITSCREEN_BARS, void, draw_bars) {
        if (!LeftRight::PaintDividers(kGame, entry_bars.m_target - entry_bars.m_offset))
            ((draw_bars_t)entry_bars.m_pOriginal)();
    }

    Halo3Entry(entry_hud_resolution, OFFSET_HALO3_PF_HUD_RESOLUTION, int, hud_resolution, int user) {
        int resolution = ((hud_resolution_t)entry_hud_resolution.m_pOriginal)(user);
        return LeftRight::HudResolution(kGame, entry_hud_resolution.m_target - entry_hud_resolution.m_offset,
                                        s_state, user, resolution);
    }

    Halo3Entry(entry_hud_layout, OFFSET_HALO3_PF_HUD_LAYOUT, const void*, hud_layout, int user, int resolution) {
        auto layout = ((hud_layout_t)entry_hud_layout.m_pOriginal)(user, resolution);
        return LeftRight::HudLayout(kGame, entry_hud_layout.m_target - entry_hud_layout.m_offset, s_state, user,
                                    layout);
    }

    Halo3Entry(entry_rt, OFFSET_HALO3_PF_RT_CREATE, __int64, create_target, void* target, int* sizes,
               const unsigned char* desc, int variant) {
        LeftRight::SizeSurface(kGame, entry_rt.m_target - entry_rt.m_offset, s_state, sizes, desc, variant);
        return ((create_target_t)entry_rt.m_pOriginal)(target, sizes, desc, variant);
    }
}
