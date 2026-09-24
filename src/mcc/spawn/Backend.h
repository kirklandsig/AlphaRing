#pragma once

// Helpers shared by the per-game spawn backends (implementation detail of mcc/spawn).

#include "Spawn.h"
#include "Command.h"

#include <cmath>
#include <string>

namespace MCC::Spawn {
    struct Vector3 { float x, y, z; };

    // Calls a function inside a game module: EngineCall<R>(game, rva, args...).
    template <typename R, typename... A>
    inline R EngineCall(int game, __int64 rva, A... args) {
        return reinterpret_cast<R(__fastcall*)(A...)>(MCC::Command::ModuleBase(game) + rva)(args...);
    }

    // Reads a global variable of a game module.
    template <typename T>
    inline T EngineGlobal(int game, __int64 rva) { return *(T*)(MCC::Command::ModuleBase(game) + rva); }

    // Blam group tags are four characters packed big-endian: 'vehi' -> 0x76656869.
    constexpr unsigned int GroupTag(const char (&s)[5]) {
        return (unsigned)s[0] << 24 | (unsigned)s[1] << 16 | (unsigned)s[2] << 8 | (unsigned)s[3];
    }

    // Where to put a spawned object: `distance` ahead of an origin along its horizontal
    // facing, raised by `height`, facing the same way.
    struct Placement {
        Vector3 position, forward, up;
        // Yaw of a character standing here looking back at the player.
        float YawToPlayer() const { return std::atan2(-forward.y, -forward.x); }
    };

    inline Placement PlaceAhead(const Vector3& origin, const Vector3& facing, float distance, float height) {
        float fx = facing.x, fy = facing.y;
        float length = std::sqrt(fx * fx + fy * fy);
        if (length < 1e-3f) { fx = 1.0f; fy = 0.0f; } else { fx /= length; fy /= length; }
        return {{origin.x + fx * distance, origin.y + fy * distance, origin.z + height}, {fx, fy, 0.0f}, {0.0f, 0.0f, 1.0f}};
    }

    inline float Distance2(const Vector3& a, const Vector3& b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    // Blam team indices: 2 human (the players' side), 3 covenant; `own` keeps the character's.
    inline short EngineTeam(Team team, short own) { return team == TeamAlly ? 2 : team == TeamEnemy ? 3 : own; }

    inline std::string SpawnResult(bool spawned, const std::string& name) {
        return (spawned ? "Spawned " : "Couldn't spawn ") + name;
    }

    // Spawn distances per category: vehicles need room, pickups land at the player's feet.
    inline float SpawnDistance(Category category) {
        switch (category) {
            case Vehicles: return 5.0f;
            case Characters: return 3.0f;
            default: return 1.5f;
        }
    }

    // "objects\characters\brute\ai\brute_captain" -> "Brute Captain": the last part of a tag
    // path, in words. Short vowel-less words are acronyms ("smg" -> "SMG", "br_ammo" -> "BR Ammo").
    inline std::string DisplayName(const char* path) {
        const char* start = path ? path : "";
        if (const char* slash = strrchr(start, '\\')) start = slash + 1;

        std::string name, word;
        auto flush = [&] {
            if (word.empty()) return;
            bool acronym = word.size() <= 4 && word.find_first_of("aeiouy") == std::string::npos;
            for (auto& c : word) c = acronym ? (char)toupper((unsigned char)c) : c;
            word[0] = (char)toupper((unsigned char)word[0]);
            if (word == "Odst") word = "ODST";
            name += (name.empty() ? "" : " ") + word;
            word.clear();
        };
        for (const char* p = start; *p; ++p) {
            if (*p == '_' || *p == ' ') flush();
            else word += *p;
        }
        flush();
        return name.empty() ? "Unnamed" : name;
    }
}
