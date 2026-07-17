// Ported from thejackbitt/megabitt01's AlphaRing fork (beta 1.3.3); see MenuConfig.h.
#include "MenuConfig.h"

#include "filesystem/Filesystem.h"
#include "log/Log.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <string>

MenuConfig g_menuConfig;

static std::string ConfigPath() {
    char exePath[MAX_PATH] = {0};
    if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH))
        return MenuConfig::k_configName;
    return (std::filesystem::path(exePath).parent_path() / MenuConfig::k_configName).string();
}

std::string MenuConfig::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

WORD MenuConfig::parseButton(const std::string& raw) {
    static const std::map<std::string, WORD> k_buttonMap = {
        {"DPAD_UP",        XINPUT_GAMEPAD_DPAD_UP},
        {"DPAD_DOWN",      XINPUT_GAMEPAD_DPAD_DOWN},
        {"DPAD_LEFT",      XINPUT_GAMEPAD_DPAD_LEFT},
        {"DPAD_RIGHT",     XINPUT_GAMEPAD_DPAD_RIGHT},
        {"START",          XINPUT_GAMEPAD_START},
        {"BACK",           XINPUT_GAMEPAD_BACK},
        {"LEFT_THUMB",     XINPUT_GAMEPAD_LEFT_THUMB},
        {"RIGHT_THUMB",    XINPUT_GAMEPAD_RIGHT_THUMB},
        {"LEFT_SHOULDER",  XINPUT_GAMEPAD_LEFT_SHOULDER},
        {"RIGHT_SHOULDER", XINPUT_GAMEPAD_RIGHT_SHOULDER},
        {"A",              XINPUT_GAMEPAD_A},
        {"B",              XINPUT_GAMEPAD_B},
        {"X",              XINPUT_GAMEPAD_X},
        {"Y",              XINPUT_GAMEPAD_Y},
    };

    std::string name = trim(raw);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    auto it = k_buttonMap.find(name);
    return (it != k_buttonMap.end()) ? it->second : 0;
}

WORD MenuConfig::parseCombo(const std::string& value) {
    WORD mask = 0;
    std::istringstream ss(value);
    std::string token;
    while (std::getline(ss, token, '+')) {
        WORD button = parseButton(token);
        // Fail the whole chord on any unknown token: silently dropping it
        // would turn a typo like START+BAKC into a bare START binding that
        // fires on ordinary pause input.
        if (button == 0)
            return 0;
        mask |= button;
    }
    return mask;
}

int MenuConfig::parseKey(const std::string& raw) {
    static const std::map<std::string, int> k_keyMap = {
        {"F1",     VK_F1},  {"F2",  VK_F2},  {"F3",  VK_F3},  {"F4",  VK_F4},
        {"F5",     VK_F5},  {"F6",  VK_F6},  {"F7",  VK_F7},  {"F8",  VK_F8},
        {"F9",     VK_F9},  {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
        {"ENTER",  VK_RETURN},
        {"ESCAPE", VK_ESCAPE},
        {"SPACE",  VK_SPACE},
        {"TAB",    VK_TAB},
    };

    std::string name = trim(raw);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    auto it = k_keyMap.find(name);
    if (it != k_keyMap.end()) return it->second;

    // Single letter A-Z
    if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z')
        return static_cast<int>(name[0]);

    return 0;
}

void MenuConfig::writeDefault(const std::string& path) {
    static const char k_defaultCfg[] =
        "# AlphaRing menu hotkey configuration\n"
        "#\n"
        "# Available Xbox controller button IDs:\n"
        "#   DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT\n"
        "#   START, BACK\n"
        "#   LEFT_THUMB, RIGHT_THUMB\n"
        "#   LEFT_SHOULDER, RIGHT_SHOULDER\n"
        "#   A, B, X, Y\n"
        "#\n"
        "# Use + to combine buttons for a combo (e.g., START+BACK)\n"
        "# Keyboard key IDs: F1-F12, ENTER, ESCAPE, SPACE, TAB, A-Z\n"
        "\n"
        "open_menu_controller=START+BACK\n"
        "open_menu_keyboard=F4\n";

    AlphaRing::Filesystem::Save(path.c_str(), k_defaultCfg, sizeof(k_defaultCfg) - 1);
}

MenuConfig MenuConfig::load() {
    MenuConfig cfg;
    auto path = ConfigPath();
    std::ifstream ifs(path);

    if (!ifs.is_open()) {
        writeDefault(path);
        return cfg;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        line = trim(line);
        if (line.empty()) continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        if (key == "open_menu_controller") {
            WORD mask = parseCombo(val);
            if (mask != 0)
                cfg.controllerComboMask = mask;
            else
                LOG_WARNING("MenuConfig: invalid controller combo '{}', keeping default", val);
        }
        else if (key == "open_menu_keyboard") {
            int vk = parseKey(val);
            if (vk != 0)
                cfg.keyboardVKey = vk;
            else
                LOG_WARNING("MenuConfig: unknown keyboard key '{}', keeping default", val);
        }
    }

    return cfg;
}
