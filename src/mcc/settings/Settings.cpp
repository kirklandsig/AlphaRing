#include "mcc/settings/Settings.h"
#include "global/Global.h"
#include "nlohmann/json.hpp"

#include <offset_mcc.h>
#include <filesystem>
#include <fstream>
#include <windows.h>

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace MCC::Settings {
    static SplitscreenConfig g_Config;

    static fs::path GetConfigPath() {
    char exePath[MAX_PATH] = {0};
    if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH))
        return "settings.json";

        fs::path dir = fs::path(exePath).parent_path();
        return dir / "settings.json";
    }


    const SplitscreenConfig& Splitscreen::Get() {
        return g_Config;
    }

    bool Splitscreen::Load() {
        fs::path path = GetConfigPath();

        if (!fs::exists(path))
            return false;

        std::ifstream file(path);
        if (!file.is_open())
            return false;

        json j;
        file >> j;

        if (!j.contains("splitscreen"))
            return false;

        auto& s = j["splitscreen"];

        if (j.contains("version"))
            g_Config.version = j["version"].get<std::string>();

        g_Config.b_override            = s.value("b_override", false);
        g_Config.player_count        = s.value("player_count", 1);
        g_Config.b_use_player0_profile = s.value("b_use_player0_profile", true);
        g_Config.b_player0_use_km      = s.value("b_player0_use_km", false);
        g_Config.b_override_profile    = s.value("b_override_profile", false);

        return true;
    }

    bool Splitscreen::Save() {
        fs::path path = GetConfigPath();

        json j;
        j["version"] = g_Config.version;

        j["splitscreen"] = {
            {"b_override", g_Config.b_override},
            {"player_count", g_Config.player_count},
            {"b_use_player0_profile", g_Config.b_use_player0_profile},
            {"b_player0_use_km", g_Config.b_player0_use_km},
            {"b_override_profile", g_Config.b_override_profile}
        };

        std::ofstream file(path, std::ios::trunc);
        if (!file.is_open())
            return false;

        file << j.dump(4);
        return true;
    }

    void Splitscreen::ApplyToRuntime() {
        auto p = AlphaRing::Global::MCC::Splitscreen();
        if (!p)
            return;

        p->b_override            = g_Config.b_override;
        p->player_count          = g_Config.player_count;
        p->b_use_player0_profile = g_Config.b_use_player0_profile;
        p->b_player0_use_km      = g_Config.b_player0_use_km;
        p->b_override_profile    = g_Config.b_override_profile;
    }

    void Splitscreen::CaptureFromRuntime() {
        auto p = AlphaRing::Global::MCC::Splitscreen();
        if (!p)
            return;

        g_Config.b_override            = p->b_override;
        g_Config.player_count        = p->player_count;
        g_Config.b_use_player0_profile = p->b_use_player0_profile;
        g_Config.b_player0_use_km      = p->b_player0_use_km;
        g_Config.b_override_profile    = p->b_override_profile;
    }
}