#include "halo3odst.h"

#include "common.h"

#include "mcc/CGameGlobal.h"
#include "mcc/spawn/Command.h"
#include <queue>
#include <mutex>

namespace Halo3ODST::Entry::World {
    std::mutex tasks_mutex;
    std::queue<std::function<void()>> tasks;

    // Runs the tasks queued before this tick; the ones they queue run on the next.
    void ExecuteTask() {
        std::queue<std::function<void()>> due;
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            due.swap(tasks);
        }
        for (; !due.empty(); due.pop()) due.front()();
    }

    void AddTask(const std::function<void()>& func) {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        tasks.push(func);
    }

    void Prologue() {
        ExecuteTask();
    }

    void Epilogue() {

    }

    Halo3ODSTEntry(entry, OFFSET_HALO3ODST_PF_WORLD, void, detour) {
        Prologue();
        ((detour_t)entry.m_pOriginal)();
        Epilogue();
    }
}

// Spawning and other engine calls must run on this (the simulation) thread.
static const bool s_registered_scheduler =
    (MCC::Command::RegisterScheduler(CGameGlobal::Halo3ODST, Halo3ODST::Entry::World::AddTask), true);
