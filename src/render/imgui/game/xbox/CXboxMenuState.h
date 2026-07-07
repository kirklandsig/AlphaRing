#pragma once

#include <string>

struct PlayerColor {
    int colors[3];
};

struct MenuState {
    int playerCount;
    bool useKM;
    int controllerIndex[4];
    int teamIndex[4];
    PlayerColor playerColors[4];
};

bool saveMenuStateBin(const MenuState& state, const std::string& path);
bool loadMenuStateBin(MenuState& state, const std::string& path);
