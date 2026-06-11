#pragma once

#include <Windows.h>
#include <Xinput.h>
#include <string>

struct MenuConfig {
    WORD controllerComboMask = XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_LEFT_THUMB;
    WORD debugComboMask      = XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_RIGHT_THUMB;
    int  keyboardVKey        = VK_F4;
    int  debugKeyboardVKey   = VK_F1;

    static constexpr const char* k_configPath = "./alpha_ring_menu.cfg";

    static MenuConfig load();

private:
    static void        writeDefault(const std::string& path);
    static WORD        parseButton(const std::string& name);
    static int         parseKey(const std::string& name);
    static std::string trim(const std::string& s);
};

extern MenuConfig g_menuConfig;
