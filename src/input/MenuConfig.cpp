#include "MenuConfig.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <string>

MenuConfig g_menuConfig;

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

static const std::map<std::string, int> k_keyMap = {
    {"F1",     VK_F1},  {"F2",  VK_F2},  {"F3",  VK_F3},  {"F4",  VK_F4},
    {"F5",     VK_F5},  {"F6",  VK_F6},  {"F7",  VK_F7},  {"F8",  VK_F8},
    {"F9",     VK_F9},  {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
    {"ENTER",  VK_RETURN},
    {"ESCAPE", VK_ESCAPE},
    {"SPACE",  VK_SPACE},
    {"TAB",    VK_TAB},
};

std::string MenuConfig::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

WORD MenuConfig::parseButton(const std::string& raw) {
    std::string name = trim(raw);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    auto it = k_buttonMap.find(name);
    return (it != k_buttonMap.end()) ? it->second : 0;
}

int MenuConfig::parseKey(const std::string& raw) {
    std::string name = trim(raw);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    auto it = k_keyMap.find(name);
    if (it != k_keyMap.end()) return it->second;

    // Single letter A-Z
    if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z')
        return static_cast<int>(name[0]);

    return VK_F4;
}

void MenuConfig::writeDefault(const std::string& path) {
    std::ofstream ofs(path);
    if (!ofs) return;

    ofs << "# AlphaRing Xbox Menu Configuration\n"
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
           "# Note: open_menu_controller and open_debug_controller must not use the same combo.\n"
           "\n"
           "open_menu_controller=START+LEFT_THUMB\n"
           "open_debug_controller=START+RIGHT_THUMB\n"
           "open_menu_keyboard=F4\n"
           "open_debug_keyboard=F1\n";
}

MenuConfig MenuConfig::load() {
    MenuConfig cfg;
    std::ifstream ifs(k_configPath);

    if (!ifs.is_open()) {
        writeDefault(k_configPath);
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

        auto parseCombo = [&](const std::string& v) -> WORD {
            WORD mask = 0;
            std::istringstream ss(v);
            std::string token;
            while (std::getline(ss, token, '+'))
                mask |= parseButton(token);
            return mask;
        };

        if (key == "open_menu_controller") {
            WORD mask = parseCombo(val);
            if (mask != 0) cfg.controllerComboMask = mask;
        }
        else if (key == "open_debug_controller") {
            WORD mask = parseCombo(val);
            if (mask != 0) cfg.debugComboMask = mask;
        }
        else if (key == "open_menu_keyboard") {
            int vk = parseKey(val);
            if (vk != 0) cfg.keyboardVKey = vk;
        }
        else if (key == "open_debug_keyboard") {
            int vk = parseKey(val);
            if (vk != 0) cfg.debugKeyboardVKey = vk;
        }
    }

    return cfg;
}
