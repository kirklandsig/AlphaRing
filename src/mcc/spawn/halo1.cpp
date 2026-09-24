// Halo CE spawn backend. Mirrors what the retail cheat_spawn_warthog / cheat_all_chars
// script functions do: object_placement_data_new + object_new for any object tag, and for
// characters actor_customize_unit (weapons) + ai_attach_free (a free, encounterless actor).

#include "Backend.h"

#include "common.h"

#include "mcc/CGameGlobal.h"

#include <offset_halo1.h>

namespace MCC::Spawn::Halo1 {
    static constexpr int kGame = CGameGlobal::Halo1;

    template <typename R, typename... A>
    static R Call(__int64 rva, A... args) { return EngineCall<R>(kGame, rva, args...); }

    struct TagIterator {
        unsigned __int64 reserved0 = 0;
        unsigned short reserved1 = 0;
        int current = -1;
        unsigned int group = 0;
        unsigned int reserved2[3] = {};
    };

    template <typename F>
    static void ForEachTag(unsigned int group, F&& f) {
        TagIterator it;
        it.group = group;
        for (int tag; (tag = Call<int>(OFFSET_HALO1_PF_TAG_ITERATOR_NEXT, &it)) != -1;) f(tag);
    }

    static const char* TagName(int tag) { return Call<const char*>(OFFSET_HALO1_PF_TAG_GET_NAME, tag); }
    static char* TagData(int tag) { return Call<char*>(OFFSET_HALO1_PF_TAG_GET, tag); }

    // actor_variant: +0x14 'unit' tag reference, whose tag index sits 12 bytes in.
    static int ActorVariantUnit(int actor_variant) { return *(int*)(TagData(actor_variant) + 0x20); }

    static int PlayerUnit(int player) {
        auto array = EngineGlobal<char*>(kGame, OFFSET_HALO1_PV_PLAYERS);
        if (array == nullptr || player < 0 || player >= kMaxPlayers) return -1;
        auto element = array + *(int*)(array + 0x34) + player * 0xC20;
        if (*(short*)element == 0) return -1; // free slot
        return *(int*)(element + 0x64);
    }

    // Returns the new object index, or -1.
    static int SpawnObject(int tag, int player, Category category, short team) {
        int unit = PlayerUnit(player);
        if (unit == -1) return -1;

        Vector3 origin, forward, up;
        Call<void>(OFFSET_HALO1_PF_OBJECT_GET_ORIGIN, unit, &origin);
        Call<void>(OFFSET_HALO1_PF_OBJECT_GET_ORIENTATION, unit, &forward, &up);
        auto place = PlaceAhead(origin, forward, SpawnDistance(category), 0.8f);

        alignas(16) char data[0x100] = {}; // object_placement_data is 0x8C bytes
        Call<void>(OFFSET_HALO1_PF_OBJECT_PLACEMENT_DATA_NEW, data, tag, -1);
        *(Vector3*)(data + 0x1C) = place.position;
        *(Vector3*)(data + 0x38) = place.forward;
        *(Vector3*)(data + 0x44) = place.up;
        if (team >= 0) *(short*)(data + 0x18) = team;
        return Call<int>(OFFSET_HALO1_PF_OBJECT_NEW, data);
    }

    static void List(Category category, std::vector<Item>& out) {
        static const unsigned int groups[kCategoryCount] = {
            GroupTag("vehi"), GroupTag("weap"), GroupTag("eqip"), GroupTag("actv")};

        ForEachTag(groups[category], [&](int tag) {
            const char* path = TagName(tag);
            if (path == nullptr) return;
            // vehicle turrets are weapon tags nobody can carry
            if (category == Weapons && strncmp(path, "vehicles\\", 9) == 0) return;
            if (category == Characters && ActorVariantUnit(tag) == -1) return;
            out.push_back({tag, DisplayName(path)});
        });
    }

    static std::string Spawn(Category category, int tag, int player, Team team) {
        std::string name = DisplayName(TagName(tag));

        if (category != Characters)
            return SpawnResult(SpawnObject(tag, player, category, -1) != -1, name);

        int unit = SpawnObject(ActorVariantUnit(tag), player, category, EngineTeam(team, -1));
        if (unit == -1) return SpawnResult(false, name);

        Call<void>(OFFSET_HALO1_PF_ACTOR_CUSTOMIZE_UNIT, tag, unit);
        Call<void>(OFFSET_HALO1_PF_AI_ATTACH_FREE, unit, tag);
        return SpawnResult(true, name);
    }

    static const Backend s_backend = {List, Spawn};
    static const bool s_registered = (RegisterBackend(kGame, &s_backend), true);
}
