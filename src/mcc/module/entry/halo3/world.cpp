#include "halo3.h"

#include "common.h"

#include "mcc/CGameGlobal.h"
#include "mcc/spawn/Command.h"

#include <queue>
#include <mutex>

namespace Halo3::Entry::World {
    std::mutex tasks_mutex;
    std::queue<std::function<void()>> tasks;

    void AddTask(const std::function<void()>& func) {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        tasks.push(func);
    }

    // Runs the tasks queued before this tick; the ones they queue run on the next.
    void ExecuteTask() {
        std::queue<std::function<void()>> due;
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            due.swap(tasks);
        }
        for (; !due.empty(); due.pop()) due.front()();
    }

    void Prologue() {
        ExecuteTask();
    }

    void Epilogue() {

    }

    Halo3Entry(entry, OFFSET_HALO3_PF_WORLD, void, detour) {
        Prologue();
        ((detour_t )entry.m_pOriginal)();
        Epilogue();
    }
}

// Spawning and other engine calls must run on this (the simulation) thread.
static const bool s_registered_scheduler =
    (MCC::Command::RegisterScheduler(CGameGlobal::Halo3, Halo3::Entry::World::AddTask), true);
