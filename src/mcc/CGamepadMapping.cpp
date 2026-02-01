#include "CGamepadMapping.h"
#include "CGameEngine.h"

static std::array<const char*, 17> button_names {
        "Left Trigger","Right Trigger",
        "Dpad Up","Dpad Down","Dpad Left","Dpad Right",
        "Start","Back",
        "Left Thumb","Right Thumb",
        "Left Shoulder","Right Shoulder",
        "A","B","X","Y",
        "None"  // Unbound
};

static std::array<const char*, 66> action_names {
        "Jump",
        "Switch Grenades 1",
        "Action",
        "Reload Right Weapon",
        "Change Weapon",
        "Melee",
        "Toggle Flashlight",
        "Throw Grenade",
        "Use Right Weapon",
        "Crouch",
        "Player Zoom",
        nullptr,
        nullptr,
        "Swap/Reload Left Weapon",
        "Sprint",
        "Banshee Bomb",
        "Player Move Forward", // H1A
        "Player Move Backward", // H1A
        "Player Move Left", // H1A
        "Player Move Right", // H1A
        "Multiplayer Scoreboard",
        "Vehicle Function 2",
        "Vehicle Function 3",
        "Use Equipment",
        "Vehicle Function 1",
        "Editor Ascend",
        "Editor Descend",
        "Drop/Pickup",
        "Thrust",
        "Precision Mode",
        "Delete",
        "Object Options",
        "Tools",
        "Play/Edit",
        "Editor Zoom",
        "Rotate",
        "TOGGLE PANEL",
        "TOGGLE INTERFACE",
        "TOGGLE FIRST/THIRD PERSON",
        "CAMERA RESET",
        "JUMP FORWARD",
        "JUMP BACK",
        "PAUSE/RESUME PLAYBACK",
        "FAST FORWARD",
        "TOGGLE FREECAM",
        "BOOST",
        "THEATER PANNING",
        "THEATER ASCEND",
        "THEATER DESCEND",
        "Use Left Weapon",
        "Theater Zoom",
        "Toggle Rotation Axes",
        "Duplicate",
        "Lock",
        "Reset",
        "Select Next Grenades 2",
        "Select Previous Grenades 2",
        "Special Action",
        "Open Loadouts Menu",
        "Toggle Display Waypoint Markers",
        "Toggle Display Waypoint Markers Alternative",
        "Push to Talk", // not sure
        "Vehicle Ascend",
        "Vehicle Descend",
        "Select Previous Grenades",
        "Select Next Grenades",
};

const std::array<const char *, 17>* CGamepadMapping::ButtonNames() {return &button_names;}

const std::array<const char *, 66>* CGamepadMapping::ActionNames() {return &action_names;}

#include <imgui.h>
#include <cstdio>
#include "input/Input.h"

// Detect which button is currently pressed on a controller
static int DetectPressedButton(int controllerIndex) {
    XINPUT_STATE state;
    if (!AlphaRing::Input::GetXInputGetState(controllerIndex, &state))
        return -1;

    // Check triggers first
    if (state.Gamepad.bLeftTrigger > 30) return CGamepadMapping::LeftTrigger;
    if (state.Gamepad.bRightTrigger > 30) return CGamepadMapping::RightTrigger;

    // Check buttons
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) return CGamepadMapping::DpadUp;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) return CGamepadMapping::DpadDown;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) return CGamepadMapping::DpadLeft;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) return CGamepadMapping::DpadRight;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) return CGamepadMapping::Start;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) return CGamepadMapping::Back;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) return CGamepadMapping::LeftThumb;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) return CGamepadMapping::RightThumb;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) return CGamepadMapping::LeftShoulder;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) return CGamepadMapping::RightShoulder;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) return CGamepadMapping::A;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) return CGamepadMapping::B;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) return CGamepadMapping::X;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) return CGamepadMapping::Y;

    return -1;
}

void CGamepadMapping::ImGuiContext() {
    char buffer[64];
    bool result = false;
    static int binding_action = -1;  // Which action is being bound (-1 = none)
    static int binding_controller = 0;  // Which controller to listen to

    // Controller selector for binding
    ImGui::PushItemWidth(150);
    ImGui::Combo("Bind Controller", &binding_controller, "Controller 1\0Controller 2\0Controller 3\0Controller 4\0");
    ImGui::PopItemWidth();
    ImGui::Separator();

    // Check for button press if binding is active
    if (binding_action >= 0) {
        int pressed = DetectPressedButton(binding_controller);
        if (pressed >= 0) {
            actions[binding_action] = static_cast<CGamepadMapping::eButton>(pressed);
            binding_action = -1;
            result = true;
        }
    }

    for (int i = 0; i < action_names.size(); ++i) {
        auto name = action_names.at(i);
        if (name == nullptr) {
            sprintf(buffer, "Action %d", i);
            name = buffer;
        }

        bool is_binding = (binding_action == i);

        if (is_binding) {
            // Show binding prompt
            ImGui::Text("%s:", name);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press button...");
            ImGui::SameLine();
            sprintf(buffer, "Cancel##%d", i);
            if (ImGui::Button(buffer)) {
                binding_action = -1;
            }
        } else {
            // Show current binding with dropdown and bind button
            ImGui::PushItemWidth(150);
            // Convert None (-1) to index 16 for dropdown display
            int value = (actions[i] == CGamepadMapping::None) ? 16 : static_cast<int>(actions[i]);
            if (ImGui::Combo(name, &value, button_names.data(), button_names.size())) {
                // Convert index 16 back to None (-1) for storage
                actions[i] = (value == 16) ? CGamepadMapping::None : static_cast<CGamepadMapping::eButton>(value);
                result = true;
            }
            ImGui::PopItemWidth();

            ImGui::SameLine();
            sprintf(buffer, "Bind##%d", i);
            if (ImGui::Button(buffer) && binding_action < 0) {
                binding_action = i;
            }
        }
    }

    if (result) {
        auto p_engine = GameEngine();
        if (p_engine) p_engine->load_setting();
    }
}
