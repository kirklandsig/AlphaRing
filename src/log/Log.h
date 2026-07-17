#pragma once

#include <spdlog.h>

namespace AlphaRing::Log {
    extern std::shared_ptr<spdlog::logger> default_logger;

    bool Init();
    bool Shutdown();
}

// Guarded: default_logger stays null if Log::Init() fails (e.g. win64 folder
// not writable), and logging must never be the thing that crashes the game.
#define LOG_INFO(...) do { if (AlphaRing::Log::default_logger) AlphaRing::Log::default_logger->info(__VA_ARGS__); } while (0)
#define LOG_ERROR(...) do { if (AlphaRing::Log::default_logger) AlphaRing::Log::default_logger->error(__VA_ARGS__); } while (0)
#define LOG_WARNING(...) do { if (AlphaRing::Log::default_logger) AlphaRing::Log::default_logger->warn(__VA_ARGS__); } while (0)
#define LOG_DEBUG(...) do { if (AlphaRing::Log::default_logger) AlphaRing::Log::default_logger->debug(__VA_ARGS__); } while (0)