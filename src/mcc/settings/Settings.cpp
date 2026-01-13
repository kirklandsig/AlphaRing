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
    static CGameManager::Profile_t g_Profiles[4];

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

        g_Config.b_override            = s.value("b_override", false);
        g_Config.player_count          = s.value("player_count", 1);
        g_Config.b_use_player0_profile = s.value("b_use_player0_profile", true);
        g_Config.b_player0_use_km      = s.value("b_player0_use_km", false);
        g_Config.b_override_profile    = s.value("b_override_profile", false);

        return true;
    }

    bool Profile::Load() {
        fs::path path = GetConfigPath();

        if (!fs::exists(path))
            return false;

            std::ifstream file(path);
            if (!file.is_open())
                return false;

            json j;
            file >> j;

            if (!j.contains("profile"))
                return false;

            auto& p = j["profile"];
            for (int i = 0; i < 4; ++i) {
                if (!p.contains(std::to_string(i)))
                    continue;

                auto& entry = p[std::to_string(i)];
                g_Profiles[i].id = entry.value("id", g_Profiles[i].id);
                g_Profiles[i].controller_index = entry.value("controller_index", g_Profiles[i].controller_index);

                auto name = entry.value("name", "");
                if (!name.empty())
                    mbstowcs(g_Profiles[i].name, name.c_str(), 1024);
            }

            return true;

    }

    bool Splitscreen::Save() {
        fs::path path = GetConfigPath();

        json j;

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

    bool Profile::Save() {
            fs::path path = GetConfigPath();

            json j;
            if (fs::exists(path)) {
                std::ifstream file(path);
                if (file.is_open()) {
                    file >> j;
                }
            }

            for (int i = 0; i < 4; ++i) {
                std::string name;
                size_t len = wcslen(g_Profiles[i].name);
                name.resize(len);
                wcstombs(name.data(), g_Profiles[i].name, len);

                j["profile"][std::to_string(i)] = {
                    {"id", g_Profiles[i].id},
                    {"controller_index", g_Profiles[i].controller_index},
                    {"name", name}
                };
            }

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

    void Profile::ApplyToRuntime() {
        for (int i = 0; i < 4; ++i) {
            auto p = CGameManager::get_profile(i);
            *p = g_Profiles[i];
        }
    }

    void Splitscreen::CaptureFromRuntime() {
        auto p = AlphaRing::Global::MCC::Splitscreen();
        if (!p)
            return;

        g_Config.b_override            = p->b_override;
        g_Config.player_count          = p->player_count;
        g_Config.b_use_player0_profile = p->b_use_player0_profile;
        g_Config.b_player0_use_km      = p->b_player0_use_km;
        g_Config.b_override_profile    = p->b_override_profile;
    }

    void Profile::CaptureFromRuntime() {
        for (int i = 0; i < 4; ++i) {
            auto p = CGameManager::get_profile(i);
            g_Profiles[i] = *p; 
        }
    }
}