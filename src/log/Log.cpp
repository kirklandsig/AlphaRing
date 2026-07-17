#include "Log.h"

#include "spdlog.h"
#include "sinks/basic_file_sink.h"

#include "common.h"

#include <filesystem>

namespace AlphaRing::Log {
    std::shared_ptr<spdlog::logger> default_logger;
    static bool console_allocated = false;

    // Best-effort: an unwritable game folder (read-only install, AV lock) must
    // never abort the game - main.cpp asserts on this result and common.h keeps
    // asserts on in Release. Degrade to console-only, or to the null-guarded
    // LOG_* macros if every sink fails, and always report success.
    bool Init() {
        std::vector<spdlog::sink_ptr> sinks;
        std::string logPathStr;

        try {
            // Get the path to the DLL (win64 folder)
            char dllPath[MAX_PATH];
            HMODULE hModule = nullptr;
            GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              (LPCSTR)&Init, &hModule);
            GetModuleFileNameA(hModule, dllPath, MAX_PATH);

            std::filesystem::path logPath = std::filesystem::path(dllPath).parent_path() / "alpharing.log";
            logPathStr = logPath.string();

            // File sink - flush on every message for crash safety
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPathStr, true);
            file_sink->set_level(spdlog::level::debug);
            sinks.push_back(file_sink);
        } catch (const std::exception& e) {
            OutputDebugStringA("AlphaRing: file logging unavailable: ");
            OutputDebugStringA(e.what());
        }

        // Console sink - allocate console for debug visibility
        console_allocated = AllocConsole();
        if (console_allocated) {
            freopen("CONIN$", "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);

            auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
            console_sink->set_level(spdlog::level::info);
            sinks.push_back(console_sink);
        }

        if (!sinks.empty()) {
            try {
                default_logger = std::make_shared<spdlog::logger>("default", sinks.begin(), sinks.end());
                default_logger->set_level(spdlog::level::debug);
                default_logger->flush_on(spdlog::level::debug);  // Flush immediately - critical for crash debugging

                spdlog::register_logger(default_logger);

                LOG_INFO("=== AlphaRing Started ===");
                if (!logPathStr.empty())
                    LOG_INFO("Log file: {}", logPathStr);
                else
                    LOG_WARNING("Log file could not be created; console only");
            } catch (const std::exception& e) {
                OutputDebugStringA("AlphaRing: Failed to initialize logging: ");
                OutputDebugStringA(e.what());
                default_logger.reset();
            }
        }

        return true;
    }

    bool Shutdown() {
        LOG_INFO("=== AlphaRing Shutdown ===");

        if (default_logger) {
            default_logger->flush();
        }

        if (console_allocated) {
            fclose(stdin);
            fclose(stdout);
            fclose(stderr);
            FreeConsole();
        }

        return true;
    }
}
