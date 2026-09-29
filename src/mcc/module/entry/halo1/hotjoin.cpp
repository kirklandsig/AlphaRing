// Halo CE hot join (mcc/splitscreen/Join.cpp): Halo CE only makes its local players as a mission loads, so a hot-join
// mission starts with all four (MCC is told of four, CGameManagerSplitscreen.cpp) and the game's local player count -
// which its views, HUD, input and cameras follow, live - is held at the player count; the others don't spawn and have
// no body. Halo CE spawns players only at the mission's starting places, which a player joining later could be far
// behind, so right after spawning they're moved to a spot a teammate stood on a moment before.
#include "halo1.h"

#include "global/Global.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace Halo1::Entry::HotJoin {
    EntryFeature("Halo CE hot join", OFFSET_HALO1_PF_GAME_START, OFFSET_HALO1_PV_LOCAL_PLAYERS,
                 OFFSET_HALO1_PV_PLAYER_COUNT, OFFSET_HALO1_PF_OBJECT_DELETE, OFFSET_HALO1_PF_PLAYER_SET_UNIT,
                 OFFSET_HALO1_PF_OBJECT_GET_ORIGIN, OFFSET_HALO1_PF_OBJECT_SET_POSITION,
                 OFFSET_HALO1_PF_OBJECT_TELEPORTED);

    constexpr int kSlots = 4;

    std::atomic<bool> s_active = false; // the running mission is a hot-join one
    int s_slots = 0;                    // local players it was started with (read on its first update)

    // Where each player stood lately on foot: every half second (the game ticks 30 times a second), a spot a step
    // from the one before at about the same height - not driving, jumping or falling. And for how many more ticks a
    // player is to be brought beside a teammate once they have a body: set while they're out of the mission, so it's
    // there when they join.
    struct Point {
        float x, y, z;
    };
    constexpr int kTrail = 8, kTrailTicks = 15, kPlaceTicks = 300;
    constexpr float kStep = 2.0f, kRise = 0.25f; // world units in half a second
    constexpr float kClear = 0.6f;               // from every player now (a Spartan is about 0.2 across)
    Point s_trail[kSlots][kTrail];               // newest first
    int s_trail_count[kSlots] = {};
    Point s_sampled[kSlots];                     // the last half-second sample, on foot or not
    bool s_has_sample[kSlots] = {};
    int s_place[kSlots] = {};
    int s_tick = 0;

    // players globals: int16 local player count +0xB4, player handle[4] +0xB8, unit[4] +0xC8
    char* Globals(__int64 module) { return *(char**)(module + OFFSET_HALO1_PV_LOCAL_PLAYERS); }
    int Handle(char* globals, int local) { return *(int*)(globals + 0xB8 + 4 * local); }
    int Unit(char* globals, int local) { return *(int*)(globals + 0xC8 + 4 * local); }

    int Joined() { return std::clamp(AlphaRing::Global::MCC::Splitscreen()->player_count, 1, kSlots); }

    int LocalIndex(char* globals, int player) {
        for (int j = 0; j < kSlots; ++j)
            if (Handle(globals, j) == player) return j;
        return -1;
    }

    Point Origin(__int64 module, int object) {
        Point at{};
        ((void (*)(int, Point*))(module + OFFSET_HALO1_PF_OBJECT_GET_ORIGIN))(object, &at);
        return at;
    }

    float Distance2(Point a, Point b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    void Sample(__int64 module, int local, int unit) {
        Point at = Origin(module, unit), before = s_sampled[local];
        bool on_foot = s_has_sample[local] && std::abs(at.z - before.z) <= kRise &&
                       Distance2({at.x, at.y, 0}, {before.x, before.y, 0}) <= kStep * kStep;
        s_sampled[local] = at;
        s_has_sample[local] = true;
        if (!on_foot) return;
        std::copy_backward(s_trail[local], s_trail[local] + kTrail - 1, s_trail[local] + kTrail);
        s_trail[local][0] = at;
        s_trail_count[local] = (std::min)(s_trail_count[local] + 1, kTrail);
    }

    void Forget(int local) {
        s_trail_count[local] = 0;
        s_has_sample[local] = false;
    }

    // The newest spot a teammate stood on that no player is on now; false while there's none (they haven't moved).
    bool Place(__int64 module, char* globals, int joiner, int joined) {
        Point now[kSlots];
        bool here[kSlots] = {};
        for (int k = 0; k < joined; ++k)
            if (k != joiner && Unit(globals, k) != -1) {
                here[k] = true;
                now[k] = Origin(module, Unit(globals, k));
            }
        for (int t = 0; t < joined; ++t) {
            if (!here[t]) continue;
            for (int n = 0; n < s_trail_count[t]; ++n) {
                Point spot = s_trail[t][n];
                bool clear = true;
                for (int k = 0; k < joined && clear; ++k) clear = !here[k] || Distance2(spot, now[k]) >= kClear * kClear;
                if (!clear) continue;
                int unit = Unit(globals, joiner);
                ((void (*)(int, Point*, void*, void*))(module + OFFSET_HALO1_PF_OBJECT_SET_POSITION))(unit, &spot, nullptr,
                                                                                                 nullptr);
                ((void (*)(int))(module + OFFSET_HALO1_PF_OBJECT_TELEPORTED))(unit);
                return true;
            }
        }
        return false;
    }

    // Each tick: the bodies of the players who aren't in the mission, the game's local player count, and bringing the
    // players who joined to the others.
    void Update(__int64 module) {
        char* globals = Globals(module);
        if (globals == nullptr) return;
        if (s_slots == 0) s_slots = std::clamp((int)*(short*)(globals + 0xB4), 1, kSlots);
        int joined = (std::min)(Joined(), s_slots);
        for (int j = joined; j < s_slots; ++j) {
            s_place[j] = kPlaceTicks;
            Forget(j);
            int unit = Unit(globals, j);
            if (unit == -1) continue;
            // the game won't delete a player's unit: the player leaves it first
            ((void (*)(int, int))(module + OFFSET_HALO1_PF_PLAYER_SET_UNIT))(Handle(globals, j), -1);
            ((void (*)(int))(module + OFFSET_HALO1_PF_OBJECT_DELETE))(unit);
        }
        *(short*)(globals + 0xB4) = (short)joined;
        *(short*)(module + OFFSET_HALO1_PV_PLAYER_COUNT) = (short)joined;

        bool sample = ++s_tick % kTrailTicks == 0;
        for (int j = 0; j < joined; ++j) {
            int unit = Unit(globals, j);
            if (unit == -1) {
                Forget(j); // the next body starts a new trail
            } else if (s_place[j] > 0) {
                if (Place(module, globals, j, joined) || --s_place[j] == 0) s_place[j] = 0;
            } else if (sample) {
                Sample(module, j, unit);
            }
        }
    }

    Halo1Entry(entry_update, OFFSET_HALO1_PF_PLAYERS_UPDATE, void, players_update, void* state) {
        if (s_active) Update(entry_update.m_target - entry_update.m_offset);
        ((players_update_t)entry_update.m_pOriginal)(state);
    }

    // Players who aren't in the mission don't spawn.
    Halo1Entry(entry_spawn, OFFSET_HALO1_PF_PLAYER_SPAWN, void, player_spawn, int player) {
        if (s_active) {
            char* globals = Globals(entry_spawn.m_target - entry_spawn.m_offset);
            int local = globals ? LocalIndex(globals, player) : -1;
            if (local >= Joined()) return;
        }
        ((player_spawn_t)entry_spawn.m_pOriginal)(player);
    }

    bool Available() { return entry_feature->Available(); }

    void Reset() {
        s_active = false;
        s_slots = s_tick = 0;
        for (int j = 0; j < kSlots; ++j) {
            s_place[j] = 0;
            Forget(j);
        }
    }
    const bool s_reset = (Halo1EntrySet()->on_remove(&Reset), true);

    // As a mission starts (halo1/graphics.cpp).
    void Start(bool on) {
        Reset();
        s_active = on && Available();
    }

    bool Active() { return s_active; }
}
