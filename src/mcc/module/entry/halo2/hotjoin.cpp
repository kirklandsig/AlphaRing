// Halo 2's local players: hot join (mcc/splitscreen/Join.cpp) and where players without a starting place come in.
//
// Hot join, as Halo CE's (halo1/hotjoin.cpp): Halo 2 only makes its local players as a mission loads, so a hot-join
// mission starts with all four (MCC is told of four, CGameManagerSplitscreen.cpp) and the game's local player count -
// which its views, HUD and input follow - is held at the player count; the others don't spawn.
//
// In a campaign each local player first spawns at one of the mission's starting places, and the missions have two:
// players 3 and 4 used to wait until players 1 and 2 walked off theirs, and a player joining later came in at the
// start of the mission, however far behind. Both come in through the co-op respawn instead, beside a teammate.
#include "halo2.h"

#include "global/Global.h"

#include <algorithm>
#include <atomic>

namespace Halo2::Entry::HotJoin {
    EntryFeature("Halo 2 hot join", OFFSET_HALO2_PF_COPY_GAME_OPTIONS, OFFSET_HALO2_PV_RESPAWN, OFFSET_HALO2_PV_PLAYERS,
                 OFFSET_HALO2_PV_SCENARIO, OFFSET_HALO2_PV_GAME_SESSION);

    constexpr int kSlots = 4;

    std::atomic<bool> s_active = false; // the running mission is a hot-join one

    // players globals: int16 local player count +8, player handle[4] +0xC (both kept by 0x69E4A0, which maps a player
    // to a local index)
    char* Globals(__int64 module) { return *(char**)(module + OFFSET_HALO2_PV_RESPAWN); }
    int Handle(char* globals, int local) { return *(int*)(globals + 0xC + 4 * local); }

    // players: data array (data at +*(+0x48)), 0x224 each: int16 flags +6 (8: spawns at a starting place), unit +0x2C
    char* Player(__int64 module, int handle) {
        char* players = *(char**)(module + OFFSET_HALO2_PV_PLAYERS);
        __int64 data = players ? *(__int64*)(players + 0x48) : 0;
        return data ? players + data + (handle & 0xFFFF) * 0x224 : nullptr;
    }

    bool Campaign(__int64 module) {
        char* session = *(char**)(module + OFFSET_HALO2_PV_GAME_SESSION);
        return session && *(int*)(session + 8) == 1;
    }

    int StartingPlaces(__int64 module) {
        char* scenario = *(char**)(module + OFFSET_HALO2_PV_SCENARIO);
        return scenario ? *(int*)(scenario + 0x100) : 0;
    }

    int Joined() { return std::clamp(AlphaRing::Global::MCC::Splitscreen()->player_count, 1, kSlots); }

    int LocalIndex(char* globals, int player) {
        for (int j = 0; j < kSlots; ++j)
            if (Handle(globals, j) == player) return j;
        return -1;
    }

    // The count the game's local players are held at: the ones who joined, once they're all mapped; until then 0, and
    // the game keeps its own count. The game counts players as it maps them (in no set order over a fresh mission's
    // first ticks) and unmaps them, so a count past the ones mapped would run on past four.
    int Held(char* globals, int joined) {
        for (int j = 0; j < joined; ++j)
            if (Handle(globals, j) == -1) return 0;
        return joined;
    }

    // Each tick: the game's local player count, and who takes the co-op respawn rather than a starting place.
    Halo2Entry(entry_update, OFFSET_HALO2_PF_PLAYERS_UPDATE, void, players_update, void* state) {
        __int64 module = entry_update.m_target - entry_update.m_offset;
        if (char* globals = Globals(module)) {
            int joined = s_active ? Joined() : kSlots;
            if (int held = s_active ? Held(globals, joined) : 0) *(short*)(globals + 8) = (short)held;
            if (Campaign(module)) {
                int places = StartingPlaces(module);
                for (int j = 0; j < kSlots; ++j) {
                    char* player = Handle(globals, j) != -1 ? Player(module, Handle(globals, j)) : nullptr;
                    if (player && *(int*)(player + 0x2C) == -1 && (j >= places || j >= joined))
                        *(unsigned short*)(player + 6) &= ~8;
                }
            }
        }
        ((players_update_t)entry_update.m_pOriginal)(state);
    }

    // Players who aren't in the mission don't spawn (a starting place, or the co-op respawn beside a teammate).
    Halo2Entry(entry_spawn, OFFSET_HALO2_PF_PLAYER_SPAWN, bool, player_spawn, int player) {
        if (s_active) {
            char* globals = Globals(entry_spawn.m_target - entry_spawn.m_offset);
            int local = globals ? LocalIndex(globals, player) : -1;
            if (local >= Joined()) return false;
        }
        return ((player_spawn_t)entry_spawn.m_pOriginal)(player);
    }

    bool Available() { return entry_feature->Available(); }

    // As a mission starts (halo2/graphics.cpp).
    void Start(bool on) { s_active = on && Available(); }

    bool Active() { return s_active; }

    void Reset() { s_active = false; }
    const bool s_reset = (Halo2EntrySet()->on_remove(&Reset), true);
}
