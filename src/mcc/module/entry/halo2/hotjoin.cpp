// Halo 2's local players: hot join (mcc/splitscreen/Join.cpp) and where players without a starting place come in.
//
// Hot join, as Halo CE's (halo1/hotjoin.cpp): Halo 2 only makes its local players as a mission loads, so a hot-join
// mission starts with all four (MCC is told of four, CGameManagerSplitscreen.cpp) and the game's local player count -
// which its views, HUD and input follow - is held at the player count; the others don't spawn.
//
// In a campaign each local player first spawns at one of the mission's starting places, and the missions have two:
// players 3 and 4 used to wait until players 1 and 2 walked off theirs, and a player joining later came in at the
// start of the mission, however far behind. Both come in through the co-op respawn instead, beside a teammate -
// where the game gives co-op respawns (not on Legendary or with the Iron skull).
//
// The players held back in a hot-join mission keep waiting for a starting place, though, as players do before their
// first spawn, until they join: one without a unit who isn't waiting for one counts as a dead teammate (0x6A16F0 sets
// players globals +5), and while there is one no checkpoint is saved (game_safe_to_save, 0x7005C0, at 0x70067E).
// Waiting so, they count as alive in the game's "every player is dead" (+4), which sends a mission whose players have
// all died back to its checkpoint, so that's worked out over the players who joined (Account).
#include "halo2.h"

#include "global/Global.h"

#include <algorithm>
#include <atomic>
#include <iterator>

namespace Halo2::Entry::HotJoin {
    EntryFeature("Halo 2 hot join", OFFSET_HALO2_PF_COPY_GAME_OPTIONS, OFFSET_HALO2_PV_RESPAWN, OFFSET_HALO2_PV_PLAYERS,
                 OFFSET_HALO2_PV_SCENARIO, OFFSET_HALO2_PF_COOP_CAMPAIGN, OFFSET_HALO2_PF_COOP_RESPAWN_ALLOWED);

    constexpr int kSlots = 4;

    std::atomic<bool> s_active = false; // the running mission is a hot-join one
    bool s_held_back[kSlots] = {};      // held back in it, and not in yet (game thread)

    // players globals: int16 local player count +8, player handle[4] +0xC (both kept by 0x69E4A0, which maps a player
    // to a local index), bytes +4 every player dead and +5 one is (0x6A16F0)
    char* Globals(__int64 module) { return *(char**)(module + OFFSET_HALO2_PV_RESPAWN); }
    int Handle(char* globals, int local) { return *(int*)(globals + 0xC + 4 * local); }

    // players: data array (data at +*(+0x48)), 0x224 each: int16 flags +6 (8: spawns at a starting place), unit +0x2C
    char* Player(__int64 module, int handle) {
        char* players = *(char**)(module + OFFSET_HALO2_PV_PLAYERS);
        __int64 data = players ? *(__int64*)(players + 0x48) : 0;
        return data ? players + data + (handle & 0xFFFF) * 0x224 : nullptr;
    }

    // Where the game gives a waiting player the co-op respawn: not on Legendary or with the Iron skull, where a co-op
    // player waiting for one counts as dead and reverts the checkpoint - there they keep their starting place.
    bool CoopRespawn(__int64 module) {
        return ((bool (*)())(module + OFFSET_HALO2_PF_COOP_CAMPAIGN))() &&
               ((bool (*)())(module + OFFSET_HALO2_PF_COOP_RESPAWN_ALLOWED))();
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

    // A local player's record, or null.
    char* Record(__int64 module, char* globals, int local) {
        int handle = Handle(globals, local);
        return handle != -1 ? Player(module, handle) : nullptr;
    }

    bool HasUnit(const char* player) { return *(const int*)(player + 0x2C) != -1; }

    // After the game's tick in a hot-join mission: whether every player is dead and whether one is, as 0x6A16F0 works
    // them out over every player, over the ones who joined (once they're all mapped; until then the game's own).
    void Account(__int64 module) {
        char* globals = s_active ? Globals(module) : nullptr;
        int joined = Joined();
        if (globals == nullptr || !Held(globals, joined)) return;
        bool all_dead = true, dead = false;
        for (int j = 0; j < joined; ++j) {
            char* player = Record(module, globals, j);
            if (player == nullptr) return;
            if (HasUnit(player) || (*(unsigned short*)(player + 6) & 8)) all_dead = false; // or waiting to spawn
            else dead = true;
        }
        globals[4] = all_dead;
        globals[5] = dead;
    }

    // Each tick: the game's local player count, and who takes the co-op respawn rather than a starting place - past the
    // starting places, or joined in the middle of a hot-join mission and not in yet; not the players held back.
    Halo2Entry(entry_update, OFFSET_HALO2_PF_PLAYERS_UPDATE, void, players_update, void* state) {
        __int64 module = entry_update.m_target - entry_update.m_offset;
        if (char* globals = Globals(module)) {
            int joined = s_active ? Joined() : kSlots;
            for (int j = 0; s_active && j < kSlots; ++j) {
                char* player = j < joined ? Record(module, globals, j) : nullptr;
                if (j >= joined) s_held_back[j] = true;
                else if (player != nullptr && HasUnit(player)) s_held_back[j] = false; // came in
            }
            if (int held = s_active ? Held(globals, joined) : 0) *(short*)(globals + 8) = (short)held;
            if (CoopRespawn(module)) {
                int places = StartingPlaces(module);
                for (int j = 0; j < joined; ++j) {
                    char* player = Record(module, globals, j);
                    if (player && !HasUnit(player) && (j >= places || s_held_back[j]))
                        *(unsigned short*)(player + 6) &= ~8;
                }
            }
        }
        ((players_update_t)entry_update.m_pOriginal)(state);
        Account(module);
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

    void Reset() {
        s_active = false;
        std::fill(std::begin(s_held_back), std::end(s_held_back), false);
    }
    const bool s_reset = (Halo2EntrySet()->on_remove(&Reset), true);

    // As a mission starts (halo2/graphics.cpp).
    void Start(bool on) {
        Reset();
        s_active = on && Available();
    }

    bool Active() { return s_active; }
}
