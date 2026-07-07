#pragma once

#include "CXboxRender.hpp"
#include "CStateMachine.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <array>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <string>

// ------------------------------------------------------------
// Persistence (unchanged)
// ------------------------------------------------------------
bool saveMenuStateBin(const MenuState& state, const std::string& path) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;

    uint8_t useKM = state.useKM ? 1 : 0;

    ofs.write(reinterpret_cast<const char*>(&state.playerCount), sizeof(state.playerCount));
    ofs.write(reinterpret_cast<const char*>(&useKM), sizeof(useKM));
    ofs.write(reinterpret_cast<const char*>(state.controllerIndex), sizeof(state.controllerIndex));
    ofs.write(reinterpret_cast<const char*>(state.teamIndex), sizeof(state.teamIndex));
    ofs.write(reinterpret_cast<const char*>(state.playerColors), sizeof(state.playerColors));

    return ofs.good();
}

bool loadMenuStateBin(MenuState& state, const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return false;

    uint8_t useKM = 0;

    ifs.read(reinterpret_cast<char*>(&state.playerCount), sizeof(state.playerCount));
    ifs.read(reinterpret_cast<char*>(&useKM), sizeof(useKM));
    state.useKM = useKM != 0;
    ifs.read(reinterpret_cast<char*>(state.controllerIndex), sizeof(state.controllerIndex));
    ifs.read(reinterpret_cast<char*>(state.teamIndex), sizeof(state.teamIndex));
    ifs.read(reinterpret_cast<char*>(state.playerColors), sizeof(state.playerColors));

    return ifs.good();
}

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
StateMachine::StateMachine(const Menu& menuInstance, const std::array<Mix_Chunk*, 6>& sounds)
    : currentState{
        menuInstance,
        sounds,
        0,
        0,
        0,
        {0,1,2,3,4,5},
        0,
        0.25f,
        Phase::Opening,
        {1, false, {0,0,0,0}, {0,1,0,1}, {{0,0,0},{0,0,0},{0,0,0},{0,0,0}}}
    },
    running(true)
{
}

// ------------------------------------------------------------
// Input layer (REPLACES SDL_Event handler)
// ------------------------------------------------------------
void StateMachine::handleInput(InputCommand cmd)
{
    if (cmd == InputCommand::Up && currentState.phase == Phase::ShiftUp) {
        finishShiftUp();
        currentState.phase = Phase::InIdle;
        currentState.time = 0;
        handleUp();
        return;
    }

    if (cmd == InputCommand::Down && currentState.phase == Phase::ShiftDown) {
        finishShiftDown();
        currentState.phase = Phase::InIdle;
        currentState.time = 0;
        handleDown();
        return;
    }

    if (!(currentState.phase == Phase::Idle ||
          currentState.phase == Phase::InIdle ||
          currentState.phase == Phase::PostOpening ||
          currentState.phase == Phase::PreClosing)) {
        return;
    }

    switch (cmd) {
        case InputCommand::Left:  handleLeft(); break;
        case InputCommand::Right: handleRight(); break;
        case InputCommand::Up:    handleUp(); break;
        case InputCommand::Down:  handleDown(); break;
        case InputCommand::Select: handleOption(); break;
        case InputCommand::Back:  handleClose(); break;
    }
}

// ------------------------------------------------------------
// Navigation
// ------------------------------------------------------------
void StateMachine::handleLeft() {
    if (currentState.phase != Phase::Idle) return;

    auto& opt = currentState.menu.pages[currentState.pageIndex]
                    .options[currentState.optionIndex];

    if (opt.type == OptionType::Increment) {
        Mix_PlayChannel(-1, currentState.sounds[2], 0);
        currentState.optionIndex = (std::max)(0, currentState.optionIndex - 2);
    } else {
        Mix_PlayChannel(-1, currentState.sounds[3], 0);
        if (currentState.pageIndex > 0) {
            currentState.phase = Phase::ShiftLeft;
            currentState.time = 0;
        }
    }
}

void StateMachine::handleRight() {
    if (currentState.phase != Phase::Idle) return;

    auto& opt = currentState.menu.pages[currentState.pageIndex]
                    .options[currentState.optionIndex];

    if (opt.type == OptionType::Decrement) {
        Mix_PlayChannel(-1, currentState.sounds[3], 0);
        currentState.optionIndex = (std::min)(
            (int)currentState.menu.pages[currentState.pageIndex].options.size() - 1,
            currentState.optionIndex + 2
        );
    } else {
        if (currentState.pageIndex >= State().menu.MAX_PAGES - 1)
            return;

        Mix_PlayChannel(-1, currentState.sounds[3], 0);
        currentState.phase = Phase::ShiftRight;
        currentState.time = 0;
    }
}

void StateMachine::handleUp() {
    if (currentState.phase == Phase::Idle) {
        auto& opt = currentState.menu.pages[currentState.pageIndex]
                        .options[currentState.optionIndex];

        if (opt.type == OptionType::Increment || opt.type == OptionType::Decrement) {
            Mix_PlayChannel(-1, currentState.sounds[5], 0);
        } else {
            Mix_PlayChannel(-1, currentState.sounds[2], 0);
            currentState.optionIndex = (std::max)(0, currentState.optionIndex - 1);
        }
    }

    if (currentState.phase == Phase::InIdle) {
        Mix_PlayChannel(-1, currentState.sounds[2], 0);
        currentState.subOptionIndex = (std::max)(0, currentState.subOptionIndex - 1);

        if (currentState.subOptionIndex < currentState.subOptionWindow[0]) {
            Mix_PlayChannel(-1, currentState.sounds[2], 0);
            currentState.phase = Phase::ShiftUp;
            currentState.time = 0;
        }
    }
}

void StateMachine::handleDown() {
    if (currentState.phase == Phase::Idle) {
        Mix_PlayChannel(-1, currentState.sounds[2], 0);

        int maxOption = (int)currentState.menu.pages[currentState.pageIndex].options.size();

        auto& opt = currentState.menu.pages[currentState.pageIndex]
                        .options[currentState.optionIndex];

        if (opt.type == OptionType::Decrement) {
            currentState.optionIndex = (std::min)(maxOption - 1, currentState.optionIndex + 3);
        } else {
            currentState.optionIndex = (std::min)(maxOption - 1, currentState.optionIndex + 1);
        }
    }

    if (currentState.phase == Phase::InIdle) {
        Mix_PlayChannel(-1, currentState.sounds[2], 0);

        int maxSub = (int)currentState.menu.pages[currentState.pageIndex]
                        .options[currentState.optionIndex]
                        .subOptions.size();

        currentState.subOptionIndex = (std::min)(maxSub - 1, currentState.subOptionIndex + 1);

        int maxWin = (int)currentState.subOptionWindow.size();
        if (currentState.subOptionIndex > currentState.subOptionWindow[maxWin - 1]) {
            currentState.phase = Phase::ShiftDown;
            currentState.time = 0;
        }
    }
}

// ------------------------------------------------------------
// Selection logic (audio replaced)
// ------------------------------------------------------------
void StateMachine::handleSubOption() {
    auto& sub = currentState.menu.pages[currentState.pageIndex]
                    .options[currentState.optionIndex]
                    .subOptions[currentState.subOptionIndex];

    Mix_PlayChannel(-1, currentState.sounds[4], 0);

    switch (sub.subOptionType) {
        case 0:
            currentState.menuState.controllerIndex[currentState.pageIndex - 1] = currentState.subOptionIndex;
            break;
        case 1:
            currentState.menuState.playerColors[currentState.pageIndex - 1].colors[0] = currentState.subOptionIndex;
            break;
        case 2:
            currentState.menuState.playerColors[currentState.pageIndex - 1].colors[1] = currentState.subOptionIndex;
            break;
        case 3:
            currentState.menuState.playerColors[currentState.pageIndex - 1].colors[2] = currentState.subOptionIndex;
            break;
        default:
            break;
    }
}

void StateMachine::handleOption() {
    auto& opt = currentState.menu.pages[currentState.pageIndex]
                    .options[currentState.optionIndex];

    Mix_PlayChannel(-1, currentState.sounds[4], 0);

    if (opt.type == OptionType::Increment && currentState.menuState.playerCount < 4)
        currentState.menuState.playerCount++;

    if (opt.type == OptionType::Decrement && currentState.menuState.playerCount > 1)
        currentState.menuState.playerCount--;

    if (opt.type == OptionType::Boolean)
        currentState.menuState.useKM = !currentState.menuState.useKM;

    if (opt.type == OptionType::TeamToggle) {
        auto& team = currentState.menuState.teamIndex[currentState.pageIndex - 1];
        team = !team;
    }

    if (currentState.phase == Phase::InIdle) {
        handleSubOption();
        currentState.phase = Phase::ShiftOut;
        currentState.time = 0;
    }

    if (opt.type == OptionType::Subpage && currentState.phase == Phase::Idle) {
        currentState.subOptionWindow = {0,1,2,3,4,5};
        currentState.phase = Phase::ShiftIn;
        currentState.time = 0;
    }
}

void StateMachine::handleClose() {
    Mix_PlayChannel(-1, currentState.sounds[1], 0);
    currentState.phase = Phase::Closing;
    currentState.time = 0;
}

void StateMachine::finishShiftUp() {
    currentState.subOptionWindow.pop_back();
    currentState.subOptionWindow.push_front(currentState.subOptionWindow.front() - 1);
}

void StateMachine::finishShiftDown() {
    currentState.subOptionWindow.pop_front();
    currentState.subOptionWindow.push_back(currentState.subOptionWindow.back() + 1);
}

// ------------------------------------------------------------
// UPDATE (unchanged logic)
// ------------------------------------------------------------
void StateMachine::update(float dt)
{
    if (currentState.time < currentState.duration)
        currentState.time += dt;

    if (currentState.time >= currentState.duration) {

        switch (currentState.phase) {

            case Phase::Opening:
                currentState.phase = Phase::Idle;
                currentState.time = 0;
                break;

            case Phase::Closing:
                running = false;
                break;

            case Phase::ShiftRight: {
                currentState.optionIndex = 0;
                currentState.phase = Phase::FadeIn;
                int maxPage = State().menu.MAX_PAGES;
                currentState.pageIndex = (std::min)(maxPage - 1, currentState.pageIndex + 1);
                break;
            }

            case Phase::ShiftLeft:
                currentState.optionIndex = 0;
                currentState.phase = Phase::FadeIn;
                currentState.pageIndex = (std::max)(0, currentState.pageIndex - 1);
                break;

            case Phase::ShiftUp:
                finishShiftUp();
                currentState.phase = Phase::InFadeIn;
                break;

            case Phase::ShiftDown:
                finishShiftDown();
                currentState.phase = Phase::InFadeIn;
                break;

            case Phase::FadeIn:
                currentState.optionIndex = 0;
                currentState.phase = Phase::Idle;
                currentState.time = 0;
                break;

            case Phase::InFadeIn:
                currentState.phase = Phase::InIdle;
                currentState.time = 0;
                break;

            case Phase::ShiftIn:
                currentState.subOptionIndex = 0;
                currentState.phase = Phase::InFadeIn;
                break;

            case Phase::ShiftOut:
                currentState.phase = Phase::FadeIn;
                break;

            default:
                break;
        }
    }
}

// ------------------------------------------------------------
// RENDER (SDL REMOVED → ImGui backend expected elsewhere)
// ------------------------------------------------------------
void StateMachine::render(int WINDOW_WIDTH, int WINDOW_HEIGHT, ImFont* font)
{
    // No SDL. Rendering should be moved into:
    // - ImGui widgets OR
    // - custom ImDrawList rendering layer

    // Example placeholder hook:
    // renderMenuImGui(drawList, currentState);
    renderMenu(WINDOW_WIDTH, WINDOW_HEIGHT, currentState, font);
}

// ------------------------------------------------------------
// Accessors
// ------------------------------------------------------------
bool StateMachine::isRunning() const {
    return running;
}

State& StateMachine::getState() {
    return currentState;
}