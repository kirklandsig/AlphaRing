#include "Log.h"

#include "spdlog.h"
#include "sinks/basic_file_sink.h"

#include "common.h"

#include <filesystem>

namespace AlphaRing::Log {
    std::shared_ptr<spdlog::logger> default_logger;
    static bool console_allocated = false;

    bool Init() {
        try {
            // Get the path to the DLL (win64 folder)
            char dllPath[MAX_PATH];
            HMODULE hModule = nullptr;
            GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              (LPCSTR)&Init, &hModule);
            GetModuleFileNameA(hModule, dllPath, MAX_PATH);

            std::filesystem::path logPath = std::filesystem::path(dllPath).parent_path() / "alpharing.log";

            // Create sinks - file sink is primary, console is optional
            std::vector<spdlog::sink_ptr> sinks;

            // File sink - always enabled, flush on every message for crash safety
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
            file_sink->set_level(spdlog::level::debug);
            sinks.push_back(file_sink);

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

            default_logger = std::make_shared<spdlog::logger>("default", sinks.begin(), sinks.end());
            default_logger->set_level(spdlog::level::debug);
            default_logger->flush_on(spdlog::level::debug);  // Flush immediately - critical for crash debugging

            spdlog::register_logger(default_logger);

            LOG_INFO("=== AlphaRing Started ===");
            LOG_INFO("Log file: {}", logPath.string());

            return true;
        } catch (const std::exception& e) {
            // Fallback if file logging fails
            OutputDebugStringA("AlphaRing: Failed to initialize logging: ");
            OutputDebugStringA(e.what());
            return false;
        }
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
