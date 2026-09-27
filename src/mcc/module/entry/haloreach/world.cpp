#include "haloreach.h"

#include "common.h"
#include "log/Log.h"
#include <queue>
#include <mutex>

namespace HaloReach::Entry::World {
    std::mutex tasks_mutex;
    std::queue<std::function<void()>> tasks;

    void AddTask(const std::function<void()>& func) {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        tasks.push(func);
    }

    void ExecuteTask() {
        std::function<void()> func = nullptr;

        std::unique_lock<std::mutex> lock(tasks_mutex);
        if (!tasks.empty()) { func = tasks.front();tasks.pop();}
        lock.unlock();

        if (func != nullptr) func();
    }

    void Prologue() {
        ExecuteTask();
    }

    void Epilogue() {

    }

    // Split screen: once bodies and dropped weapons pile up (about 440 objects with 4 players, 495 with 2, on The
    // Package), Reach stops drawing Spartans and first-person weapons in the later views, or draws them black or
    // over-bright - the last view first. Its collector only starts at 120 waiting objects and stops at 115, which
    // leaves that pile, so with 2+ local players start and stop it lower. Decided before every collection: games
    // with players from other machines keep Reach's numbers, since every machine has to collect the same objects.
    void SetGarbageLimits(__int64 module) {
        unsigned index = *(unsigned*)(module + OFFSET_HALOREACH_DAT_TLS_INDEX);
        char* tls = *(char**)(__readgsqword(0x58) + index * 8ull);
        char* players = tls ? *(char**)(tls + OFFSET_HALOREACH_TLS_PLAYERS) : nullptr;
        int local = ((int (*)())(module + OFFSET_HALOREACH_PF_GET_SPLITSCREEN_PLAYER_COUNT))();
        bool split = players != nullptr && local >= 2 && *(int*)(players + 0x48) == local;
        char start = !split ? 0x78 : local > 2 ? 40 : 60;
        char stop = start - 5;
        char* site_start = (char*)(module + OFFSET_HALOREACH_V_GARBAGE_COLLECT_START);
        char* site_stop = (char*)(module + OFFSET_HALOREACH_V_GARBAGE_COLLECT_STOP);
        if (*site_start == start && *site_stop == stop) return;
        DWORD protect;
        if (VirtualProtect(site_stop, site_start - site_stop + 1, PAGE_EXECUTE_READWRITE, &protect)) {
            *site_start = start;
            *site_stop = stop;
            VirtualProtect(site_stop, site_start - site_stop + 1, protect, &protect);
            LOG_INFO("Reach: garbage collection starts above {} waiting objects, stops at {}", (int)start, (int)stop);
        }
    }

    HaloReachEntry(collector_entry, OFFSET_HALOREACH_PF_COLLECT_GARBAGE, void, collector_detour) {
        SetGarbageLimits(collector_entry.m_target - collector_entry.m_offset);
        ((collector_detour_t)collector_entry.m_pOriginal)();
    }

    HaloReachEntry(entry, OFFSET_HALOREACH_PF_WORLD, void, detour) {
        Prologue();
        ((detour_t)entry.m_pOriginal)();
        Epilogue();
    }
}
