#include "LeftRight.h"

#include "common.h"

#include "mcc/CGameGlobal.h"
#include "mcc/module/CModule.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <iterator>

namespace MCC::Splitscreen::LeftRight {
    namespace Store = AlphaRing::SplitscreenConfigStore;

    namespace {
        // The table entries the layout owns: 2-player slots 0-1 and 3-player slots 0-2.
        struct Slot { int players, slot; };
        constexpr Slot kSlots[] = {{2, 0}, {2, 1}, {3, 0}, {3, 1}, {3, 2}};
        static_assert(std::size(kSlots) == sizeof(State::saved) / sizeof(State::saved[0]));

        // The render-target variant (the table's "resolution") the full-height halves draw into. The stock
        // two-player layout uses it too, so no other view shares its surface.
        constexpr int kHalfSurface = 3;

        // A screen rectangle in the engines' order.
        struct Rect { short top, left, bottom, right; };

        std::atomic<int> s_classic_side_by_side{-1}; // the Halo CE / Halo 2 game whose mission started side by side

        bool ClassicGame(int game) { return game == CGameGlobal::Halo1 || game == CGameGlobal::Halo2; }

        Store::LayoutEntry* Entry(const Gen3& game, __int64 module, const Slot& s) {
            return (Store::LayoutEntry*)(module + game.table) + s.players * Store::SLOT_COUNT + s.slot;
        }

        int Players(const Gen3& game, __int64 module) { return ((int (*)())(module + game.player_count))(); }

        const int* Screen(const Gen3& game, __int64 module) { return (const int*)(module + game.screen); }

        // A descriptor size in pixels, as the pool computes it: flag bit 0 makes the value a divisor of the
        // screen size, rounded half up; otherwise it is the size itself.
        int Pixels(const Gen3& game, float value, int screen, unsigned flags) {
            if (!(flags & 1)) return (int)value;
            float divided = (float)screen / (std::max)(value, 1.0f);
            return game.double_rounding ? (int)std::floor((double)divided + 0.5) : (int)std::floor(divided + 0.5f);
        }
    }

    bool Chosen() {
        return Store::GetTwoPlayerLayout() == Store::TwoPlayerLayout::LeftRight;
    }

    bool Active(int player_count) {
        return Store::ResolveActiveLayout(player_count) == Store::ActiveLayout::LeftRight;
    }

    bool Supports(int game, int player_count) {
        switch (game) {
            case CGameGlobal::Halo1: // keeps its own three-player layout (module/entry/halo1/splitscreen.cpp)
                return player_count == 2;
            case CGameGlobal::Halo2: case CGameGlobal::Halo3: case CGameGlobal::Halo4:
            case CGameGlobal::Halo3ODST: case CGameGlobal::HaloReach:
                return player_count == 2 || player_count == 3;
            default:
                return false;
        }
    }

    bool OnScreen(int game, int player_count) {
        return Supports(game, player_count) && (ClassicGame(game) ? s_classic_side_by_side == game : Chosen());
    }

    void StartClassicMission(int game, bool side_by_side) { s_classic_side_by_side = side_by_side ? game : -1; }

    bool WaitsForNextMission(int game, int player_count) {
        return ClassicGame(game) && Supports(game, player_count) && Chosen() != (s_classic_side_by_side == game);
    }

    void Choose(bool left_right) {
        auto reach = MCC::Module::GetSubModule(MCC::Module::MODULE_HALOREACH);
        Store::SetTwoPlayerLayout(left_right ? Store::TwoPlayerLayout::LeftRight : Store::TwoPlayerLayout::TopBottom,
                                  reach->info().hModule);
        // Reach's store writes the stock entries back over any black-bar patch the player turned on.
        if (!left_right) reach->patches()->apply();
    }

    void Frame(const Gen3& game, __int64 module, State& state) {
        if (state.module != module) {
            state.module = module;
            state.table_left_right = false;
        }

        if (Chosen()) {
            for (size_t i = 0; i < std::size(kSlots); ++i) {
                auto entry = Entry(game, module, kSlots[i]);
                if (!state.table_left_right) state.saved[i] = *entry;
                auto& want = Store::LeftRightEntry(kSlots[i].players, kSlots[i].slot);
                if (memcmp(entry, &want, sizeof(want)) != 0) CPatch::apply(entry, &want, sizeof(want));
            }
            state.table_left_right = true;
        } else if (state.table_left_right) {
            for (size_t i = 0; i < std::size(kSlots); ++i)
                CPatch::apply(Entry(game, module, kSlots[i]), &state.saved[i], sizeof(state.saved[i]));
            state.table_left_right = false;
        }

        // The pool sizes its surfaces once, so a switch rebuilds it: the same release/init pair the
        // engine's own resize path runs right before this render call, minus the swap-chain resize.
        unsigned generation = Store::GetLayoutGeneration();
        if (state.built != generation) {
            ((void (*)())(module + game.pool_release))();
            ((void (*)())(module + game.pool_init))();
            state.built = generation;
        }
    }

    bool PaintDividers(const Gen3& game, __int64 module) {
        if (!Chosen()) return false;
        int players = Players(game, module);
        if (!Active(players)) return false;

        // The stock dividers are 2px bands at the screen's middle; these are the same bands turned upright,
        // plus one across the right half, where players 2 and 3 share it.
        if (game.before_painting) game.before_painting(module);
        auto fill = (void (*)(Rect*, unsigned))(module + game.fill_rect);
        auto screen = Screen(game, module);
        short w = (short)screen[0], h = (short)screen[1], half_w = w >> 1, half_h = h >> 1;

        Rect vertical{0, (short)(half_w - 1), h, (short)(w - half_w + 1)};
        fill(&vertical, 0xFF000000);
        if (players == 3) {
            Rect right{(short)(half_h - 1), half_w, (short)(h - half_h + 1), w};
            fill(&right, 0xFF000000);
        }
        return true;
    }

    int HudResolution(const Gen3& game, __int64 module, State& state, int user, int resolution) {
        bool half = resolution == 1 || resolution == 5;
        bool tall = half && Chosen() && Active(Players(game, module));
        if (user >= 0 && user < 4) state.tall_hud[user] = tall;
        return tall ? (resolution == 1 ? 4 : 6) : resolution;
    }

    const void* HudLayout(const Gen3& game, __int64 module, State& state, int user, const void* layout) {
        if (layout == nullptr || user < 0 || user >= 4 || !state.tall_hud[user]) return layout;
        auto screen = Screen(game, module);
        auto copy = state.hud_layouts[user];
        memcpy(copy, layout, (std::min)(game.hud_layout_size, kMaxHudLayoutSize));
        auto canvas = (int*)(copy + game.hud_canvas);
        if (screen[0] > 1) canvas[1] = (int)((float)canvas[0] * screen[1] / (screen[0] / 2) + 0.5f);
        return copy;
    }

    bool ResizesHalfSurface(State& state, int variant) {
        state.built = Store::GetLayoutGeneration();
        return Chosen() && (variant & 3) == kHalfSurface;
    }

    void SizeSurface(const Gen3& game, __int64 module, State& state, int* sizes, const unsigned char* desc,
                     int variant) {
        if (!ResizesHalfSurface(state, variant) || !sizes || !desc) return;

        unsigned flags = *(const unsigned*)desc;
        if (!(flags & 0x20)) return; // not sized per view variant

        auto screen = Screen(game, module);
        auto field = [&](int offset) { return *(const float*)(desc + offset); };
        int w1 = Pixels(game, field(0x08), screen[0], flags), h1 = Pixels(game, field(0x0C), screen[1], flags);
        int w2 = Pixels(game, field(0x20), screen[0], flags), h2 = Pixels(game, field(0x24), screen[1], flags);
        bool same = (flags & 6) == 2; // the second pair copies the first

        int* pair1 = sizes + 6;
        int* pair2 = sizes + 1;
        if (pair1[0] != w1 * 3 / 4 || pair1[1] != h1 / 2 ||
            (!same && (pair2[0] != w2 * 3 / 4 || pair2[1] != h2 / 2))) {
            static bool logged;
            if (!logged) {
                logged = true;
                LOG_WARNING("Left/Right split: render target {}x{} does not match its descriptor, left stock",
                            pair1[0], pair1[1]);
            }
            return;
        }

        pair1[0] = w1 / 2;
        pair1[1] = h1;
        pair2[0] = same ? pair1[0] : w2 / 2;
        pair2[1] = same ? pair1[1] : h2;
    }
}
