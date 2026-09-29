// The Players window: couch-style joining. Every controller answers here directly (the overlay blocks game input
// while it's open): A joins the first free slot, B leaves, Start saves and closes. It opens by itself when the
// overlay is opened at MCC's menus, where the player count and controllers can still change before a mission. In a
// game it shows the lineup: the game reads the controllers live, so players only join there with hot join (a
// controller no player uses presses A), and only in Halo CE does the last player leave (B).
#include "Splitscreen.h"

#include "common.h"

#include "global/Global.h"
#include "input/Input.h"
#include "mcc/CGameManager.h"
#include "mcc/hud/Hud.h"
#include "mcc/mcc.h"
#include "mcc/settings/Settings.h"

#include "imgui.h"

#include <algorithm>
#include <atomic>
#include <cstdio>

namespace MCC::Splitscreen {
    static constexpr int kSlots = 4;
    static constexpr int kNone = 4; // profile controller_index: no controller
    static constexpr float kDefaultHues[kSlots] = {210.0f, 0.0f, 120.0f, 50.0f}; // blue, red, green, yellow

    static bool s_show = false;
    static bool s_auto_open = true;   // open with the overlay at MCC's menus
    static int s_last_frame = -2;     // the overlay's last frame here; the overlay only draws while shown
    static WORD s_last_buttons[kSlots] = {};
    static int s_last_poll = -2;      // the frame the pads were last read here

    static bool KeyboardSlot() {
        auto setting = AlphaRing::Global::MCC::Splitscreen();
        return setting && setting->b_player0_use_km;
    }

    static int SlotOf(int pad) {
        auto setting = AlphaRing::Global::MCC::Splitscreen();
        for (int i = KeyboardSlot() ? 1 : 0; i < setting->player_count; ++i)
            if (auto profile = CGameManager::get_profile(i); profile && profile->controller_index == pad) return i;
        return -1;
    }

    static void SavePlayers() {
        MCC::Settings::Splitscreen::CaptureFromRuntime();
        MCC::Settings::Splitscreen::Save();
        MCC::Settings::Profile::CaptureFromRuntime();
        MCC::Settings::Profile::Save();
    }

    // The buttons newly down on `pad` since `last` (kept up to date). Unless `reading` (the pads were read the frame
    // before) there are none, so buttons held from before aren't presses.
    static WORD Pressed(int pad, WORD& last, bool reading) {
        XINPUT_STATE state{};
        WORD buttons = AlphaRing::Input::GetXInputGetState(pad, &state) ? state.Gamepad.wButtons : 0;
        WORD pressed = reading ? buttons & ~last : 0;
        last = buttons;
        return pressed;
    }

    // The first player without a controller gets it, else a new player: their slot, or -1. The game reads these live
    // in a mission (hot join), so the controller is in place before the count makes them a player.
    static int Join(int pad) {
        auto setting = AlphaRing::Global::MCC::Splitscreen();
        if (SlotOf(pad) >= 0) return -1;
        int slot = KeyboardSlot() ? 1 : 0;
        while (slot < setting->player_count && CGameManager::get_profile(slot)->controller_index != kNone) ++slot;
        if (slot >= kSlots) return -1;
        CGameManager::get_profile(slot)->controller_index = pad;
        std::atomic_thread_fence(std::memory_order_release);
        setting->player_count = std::max(setting->player_count, slot + 1);
        if (setting->player_count > 1) setting->b_override = true;
        return slot;
    }

    // Everyone after the leaving player moves up a slot, keeping their controller.
    static bool Leave(int slot) {
        auto setting = AlphaRing::Global::MCC::Splitscreen();
        if (slot < 0 || setting->player_count <= 1) return false;
        for (int i = slot; i + 1 < setting->player_count; ++i)
            CGameManager::get_profile(i)->controller_index = CGameManager::get_profile(i + 1)->controller_index;
        CGameManager::get_profile(setting->player_count - 1)->controller_index = kNone;
        --setting->player_count;
        return true;
    }

    // ---- hot join (Splitscreen::HotJoinActive): the lineup changes during a mission and is saved once the map stops
    // running (saving takes a couple of frames)
    static WORD s_hot_buttons[kSlots] = {};
    static bool s_hot_polling = false; // polled the frame before
    static bool s_hot_unsaved = false;

    void HotJoinPoll() {
        if (s_hot_unsaved && !CGameManager::running()) {
            s_hot_unsaved = false;
            SavePlayers();
        }
        bool reading = s_hot_polling;
        s_hot_polling = HotJoinActive() && AlphaRing::Global::MCC::Splitscreen()->player_count < kSlots;
        if (!s_hot_polling) return;
        for (int pad = 0; pad < kSlots; ++pad) {
            if (SlotOf(pad) >= 0) {
                s_hot_buttons[pad] = 0xFFFF; // a player's pad; once free it needs a fresh press
                continue;
            }
            int slot = Pressed(pad, s_hot_buttons[pad], reading) & XINPUT_GAMEPAD_A ? Join(pad) : -1;
            if (slot < 0) continue;
            s_hot_unsaved = true;
            LOG_INFO("Hot join: controller {} joins as player {}", pad + 1, slot + 1);
        }
    }

    // A Halo CE hot-join mission: the last player leaves.
    static void HotJoinLeave(int slot) {
        auto setting = AlphaRing::Global::MCC::Splitscreen();
        if (!HotJoinLeaves() || slot <= 0 || slot != setting->player_count - 1 || !Leave(slot)) return;
        s_hot_unsaved = true;
        LOG_INFO("Hot join: player {} leaves", slot + 1);
    }

    static void PollPads(bool changes) {
        int frame = ImGui::GetFrameCount();
        bool reading = frame == s_last_poll + 1; // buttons held from before (the overlay's Start+Back) aren't presses
        s_last_poll = frame;
        for (int pad = 0; pad < kSlots; ++pad) {
            WORD pressed = Pressed(pad, s_last_buttons[pad], reading);
            if (changes && (pressed & XINPUT_GAMEPAD_A)) {
                if (Join(pad) >= 0) SavePlayers();
            } else if (changes && (pressed & XINPUT_GAMEPAD_B)) {
                if (Leave(SlotOf(pad))) SavePlayers();
            } else if (pressed & XINPUT_GAMEPAD_B) HotJoinLeave(SlotOf(pad));
            else if ((pressed & XINPUT_GAMEPAD_START) && SlotOf(pad) >= 0) {
                s_show = false;
                AlphaRing::Global::Global()->show_imgui = false;
            }
        }
    }

    static ImU32 SlotColor(int slot) {
        auto& hud = MCC::Hud::Player(slot);
        return MCC::Hud::HueColor(hud.recolor ? hud.hue : kDefaultHues[slot]);
    }

    static void Card(int slot, bool changes) {
        auto setting = AlphaRing::Global::MCC::Splitscreen();
        auto profile = CGameManager::get_profile(slot);
        bool joined = slot < setting->player_count;
        ImVec2 size(220, 120);
        ImVec2 at = ImGui::GetCursorScreenPos();
        auto draw = ImGui::GetWindowDrawList();
        ImU32 color = SlotColor(slot);
        draw->AddRectFilled(at, ImVec2(at.x + size.x, at.y + size.y), joined ? (color & 0x60FFFFFF) : IM_COL32(40, 40, 40, 160), 8.0f);
        draw->AddRect(at, ImVec2(at.x + size.x, at.y + size.y), joined ? color : IM_COL32(90, 90, 90, 255), 8.0f, 0, 3.0f);

        char line[64];
        std::snprintf(line, sizeof(line), "PLAYER %d", slot + 1);
        draw->AddText(ImVec2(at.x + 14, at.y + 12), joined ? color : IM_COL32(160, 160, 160, 255), line);
        const char* status;
        if (!joined) {
            status = changes ? "Press A to join" : "Free";
        } else if (slot == 0 && KeyboardSlot()) {
            status = "Keyboard & mouse";
        } else if (profile->controller_index >= kNone) {
            status = changes ? "No controller - press A" : "No controller";
        } else {
            XINPUT_STATE state{};
            bool connected = AlphaRing::Input::GetXInputGetState(profile->controller_index, &state);
            std::snprintf(line, sizeof(line), "Controller %d%s", profile->controller_index + 1,
                          connected ? "" : " (disconnected)");
            status = line;
        }
        draw->AddText(ImVec2(at.x + 14, at.y + 48), IM_COL32(230, 230, 230, 255), status);
        if (changes && joined && !(slot == 0 && KeyboardSlot()))
            draw->AddText(ImVec2(at.x + 14, at.y + 84), IM_COL32(170, 170, 170, 255), "B to leave");
        ImGui::Dummy(size);
    }

    void JoinContext() {
        int frame = ImGui::GetFrameCount();
        if (frame != s_last_frame + 1 && s_auto_open && !MCC::IsInGame()) s_show = true; // the overlay just opened
        s_last_frame = frame;

        if (ImGui::BeginMainMenuBar()) {
            ImGui::MenuItem("Players", nullptr, &s_show);
            ImGui::EndMainMenuBar();
        }
        if (!s_show) return;

        bool changes = !MCC::IsInGame();
        PollPads(changes);
        ImGui::SetNextWindowSize(ImVec2(960, 250), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Players", &s_show, ImGuiWindowFlags_NoCollapse)) {
            for (int slot = 0; slot < kSlots; ++slot) {
                if (slot) ImGui::SameLine();
                Card(slot, changes);
            }
            if (changes) ImGui::TextDisabled("A: join   B: leave   Start: done");
            else if (HotJoinLeaves())
                ImGui::TextDisabled("Hot join: A on a free controller joins, B leaves (the last player)   Start: done");
            else if (HotJoinActive())
                ImGui::TextDisabled("Hot join: A on a free controller joins; players leave at MCC's menus   Start: done");
            else ImGui::TextDisabled("In a game: players join and leave at MCC's menus   Start: done");
            ImGui::SameLine(ImGui::GetWindowWidth() - 330);
            ImGui::Checkbox("Open at MCC's menus", &s_auto_open);
        }
        ImGui::End();
    }
}
