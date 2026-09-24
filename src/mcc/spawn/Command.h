#pragma once

#include <functional>
#include <string>
#include <vector>

// Game-thread execution for AlphaRing features (spawning).
//
// Engines with a simulation-tick hook register a Scheduler that runs a task on that thread
// (Halo 3 and ODST, whose console scripts compile on another thread). For the others, MCC
// hands every "HS: <script>" string it receives through the engine interface to the game's
// console-script compiler on the game thread; that compiler is hooked
// (module/entry/*/console.cpp) and scripts starting with "@ar " are dispatched to the fixed
// handlers registered here instead of being compiled. Only the overlay posts commands.
namespace MCC::Command {
    using Args = std::vector<std::string>;
    using Handler = void (*)(int game, const Args& args);

    // Queue a command for the current game's thread. Returns false when no game is running.
    bool Post(const char* format, ...);

    // Register a handler for "@ar <verb> ...". Handlers run on the game thread.
    void Register(const char* verb, Handler handler);

    // Register how to run a task on `game`'s simulation thread.
    using Scheduler = void (*)(const std::function<void()>& task);
    void RegisterScheduler(int game, Scheduler scheduler);

    // Run `task` on `game`'s simulation thread at the next tick, unless another map has loaded
    // by then. False without a scheduler.
    bool Schedule(int game, const std::function<void()>& task);

    // Called by the console hooks; true when `text` was an AlphaRing command.
    bool Dispatch(int game, const char* text);

    // The loaded module base of `game` (0 when not loaded).
    __int64 ModuleBase(int game);
}
