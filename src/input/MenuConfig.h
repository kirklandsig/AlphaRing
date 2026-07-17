// Configurable menu hotkeys, ported from thejackbitt/megabitt01's AlphaRing
// fork (https://github.com/megabitt01/AlphaRing, beta 1.3.3) and adapted:
// our fork has a single overlay menu, so the debug-menu bindings were dropped
// and the defaults match this fork's existing hotkeys (F4, START+BACK).
#pragma once

#include <Windows.h>
#include <Xinput.h>
#include <string>

struct MenuConfig {
    WORD controllerComboMask = XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK;
    int  keyboardVKey        = VK_F4;

    // Stored next to the game exe (like settings.json), not the CWD.
    static constexpr const char* k_configName = "alpha_ring_menu.cfg";

    static MenuConfig load();

private:
    static void        writeDefault(const std::string& path);
    static WORD        parseButton(const std::string& name);
    static WORD        parseCombo(const std::string& value);
    static int         parseKey(const std::string& name);
    static std::string trim(const std::string& s);
};

extern MenuConfig g_menuConfig;
