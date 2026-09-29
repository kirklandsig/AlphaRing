#include "Splitscreen.h"

#include "mcc/settings/Settings.h"

#include "common.h"

#include "global/Global.h"

#include <offset_mcc.h>

#include "../CGameManager.h"
#include "mcc/CGameGlobal.h"
#include "LeftRight.h"
#include "mcc/mcc.h"

#include <atomic>

namespace Halo1::Entry::Anniversary { bool Available(); }
namespace Halo2::Entry::Anniversary { bool Available(); }
namespace Halo1::Entry::HotJoin {
    bool Available();
    bool Active();
}
namespace Halo2::Entry::HotJoin {
    bool Available();
    bool Active();
}

namespace MCC::Splitscreen {
    DefDetourFunction(__int64, __fastcall, get_index_by_xuid, void* a1, __int64 xuid) {
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        if (!p_setting->b_override)
            return ppOriginal_get_index_by_xuid(a1, xuid);

        return CGameManager::get_index(xuid);
    }

    // todo:: let other players have the ability to pause the game

    bool Initialize() {
        bool result;

        // fix: changing team freeze the game
        result = AlphaRing::Hook::Detour({
            {OFFSET_MCC_PF_GET_INDEX_BY_XUID, OFFSET_MCC_WS_PF_GET_INDEX_BY_XUID, get_index_by_xuid, (void**)&ppOriginal_get_index_by_xuid},
        });

        assertm(result, "MCC:Splitscreen: failed to hook");

        return true;
    }

    // A saved on/off choice. It's read on the render and worker threads, so it's kept here rather than read from the
    // config store each time: -1 (off) until the store has loaded, then read once.
    struct SavedChoice {
        const char* key;
        std::atomic<int> value{-1};

        bool Get() {
            int chosen = value;
            if (chosen < 0 && AlphaRing::SplitscreenConfigStore::Loaded()) {
                float saved = 0.0f;
                value = chosen = AlphaRing::SplitscreenConfigStore::Get(-1, key, saved) && saved != 0.0f;
            }
            return chosen > 0;
        }

        void Set(bool on) {
            value = on;
            AlphaRing::SplitscreenConfigStore::Set(-1, key, on ? 1.0f : 0.0f);
        }
    };

    SavedChoice g_anniversary_quad{"anniversary_quad"};
    std::atomic<bool> g_anniversary_quad_active{false}; // the current mission's

    bool AnniversaryQuadChosen() { return g_anniversary_quad.Get(); }
    void ChooseAnniversaryQuad(bool on) { g_anniversary_quad.Set(on); }
    bool AnniversaryQuadActive() { return g_anniversary_quad_active; }

    SavedChoice g_hot_join{"hot_join"};

    static void ChooseHotJoin(bool on) {
        g_hot_join.Set(on);
        if (on) AlphaRing::Global::MCC::Splitscreen()->b_override = true; // the slots are AlphaRing's
    }

    bool HotJoinOn() { return AlphaRing::Global::MCC::Splitscreen()->b_override && g_hot_join.Get(); }

    // How a game takes a player joining in the middle of a mission. Halo 3, ODST, Reach and Halo 4 make the player MCC
    // signs in. Halo CE and Halo 2 make their players as a mission loads, so their modules keep all four and hold back
    // the ones who haven't joined (module/entry/halo1/hotjoin.cpp, halo2/hotjoin.cpp): whether this build has all the
    // module's addresses, and whether the running mission started with it.
    struct Reserving {
        bool (*available)();
        bool (*active)();
    };

    static const Reserving* ReservingFor(int game) {
        static const Reserving halo1{Halo1::Entry::HotJoin::Available, Halo1::Entry::HotJoin::Active};
        static const Reserving halo2{Halo2::Entry::HotJoin::Available, Halo2::Entry::HotJoin::Active};
        return game == CGameGlobal::Halo1 ? &halo1 : game == CGameGlobal::Halo2 ? &halo2 : nullptr;
    }

    static bool SignsIn(int game) {
        return game == CGameGlobal::Halo3 || game == CGameGlobal::Halo3ODST || game == CGameGlobal::HaloReach ||
               game == CGameGlobal::Halo4;
    }

    // A running map takes joins (not one that is still loading).
    bool HotJoinActive() {
        if (!MCC::Ready() || !HotJoinOn() || !MCC::IsInGame() || !CGameManager::running()) return false;
        auto p_global = GameGlobal();
        if (p_global == nullptr) return false;
        auto reserving = ReservingFor(p_global->current_game);
        return reserving ? reserving->active() : SignsIn(p_global->current_game);
    }

    // Players leave in the middle of a Halo CE mission only: its module takes the leaver's Spartan away, and the games
    // that sign players in end the mission when MCC signs one out.
    bool HotJoinLeaves() { return HotJoinActive() && GameGlobal()->current_game == CGameGlobal::Halo1; }

    int HotJoinSlots(int player_count) {
        auto p_global = GameGlobal();
        auto reserving = p_global ? ReservingFor(p_global->current_game) : nullptr;
        return reserving && reserving->available() && HotJoinOn() ? 4 : player_count;
    }

    // Halo CE and Halo 2 take the choice as a mission starts, and MCC ends one whose reserved players go away: in one
    // of their games it's changed at MCC's menus.
    static bool HotJoinLocked() {
        auto p_global = GameGlobal();
        return MCC::IsInGame() && p_global && ReservingFor(p_global->current_game);
    }

    bool AnniversaryQuadGame(int game) { return game == CGameGlobal::Halo1 || game == CGameGlobal::Halo2; }

    // The game's quad mode has all its offsets in this MCC build (always in the one they were written for).
    static bool AnniversaryQuadAvailable(int game) {
        return game == CGameGlobal::Halo1 ? Halo1::Entry::Anniversary::Available() : Halo2::Entry::Anniversary::Available();
    }

    ClassicGraphicsScope::ClassicGraphicsScope(unsigned char* game_options, int game) {
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        bool left_right = p_setting->b_override && game_options != nullptr && LeftRight::Chosen() &&
                          LeftRight::Supports(game, p_setting->player_count);
        LeftRight::StartClassicMission(game, left_right);
        g_anniversary_quad_active = false;
        if (!p_setting->b_override || game_options == nullptr) return;
        // The 3-4 player mode is set up for every mission (it runs with 3-4 players while Anniversary is on screen):
        // Back switches Halo CE and Halo 2 to Anniversary graphics in the middle of one, their own renderers draw only
        // two views, and a hot join can bring a third player. The choice is whether missions start in Anniversary.
        bool available = AnniversaryQuadGame(game) && AnniversaryQuadAvailable(game);
        g_anniversary_quad_active = available;
        if (p_setting->player_count <= 2 && !left_right) return;
        bool quad = available && !left_right && AnniversaryQuadChosen();
        bool anniversary = game_options[0] & 1;
        if (!anniversary) return;
        if (quad) {
            LOG_INFO("Splitscreen: {} players in Anniversary graphics (experimental)", p_setting->player_count);
            return;
        }

        LOG_INFO("Splitscreen: {} players{}, starting in Classic graphics", p_setting->player_count,
                 left_right ? " side by side" : "");
        m_options = game_options;
        m_saved = game_options[0];
        game_options[0] &= ~1;
    }

    ClassicGraphicsScope::~ClassicGraphicsScope() {
        if (m_options) m_options[0] = m_saved;
    }
}

#include "imgui.h"
#include "mcc/mcc.h"
#include "input/Input.h"
#include "log/Log.h"

#include <string>

namespace MCC::Splitscreen {
    void RealContext();

    // Detect which controller (0-3) has any button/trigger pressed, returns -1 if none
    static int DetectActiveController() {
        for (int i = 0; i < 4; i++) {
            XINPUT_STATE state;
            if (AlphaRing::Input::GetXInputGetState(i, &state)) {
                if (state.Gamepad.wButtons != 0 ||
                    state.Gamepad.bLeftTrigger > 30 ||
                    state.Gamepad.bRightTrigger > 30) {
                    return i;
                }
            }
        }
        return -1;
    }

    // Binding state: which player slot is waiting for controller input (-1 = none)
    static int s_binding_player = -1;

    void ImGuiContext() {
        static bool show_splitscreen;

        JoinContext();

        if (ImGui::BeginMainMenuBar()) {
            ImGui::MenuItem("Splitscreen", nullptr, &show_splitscreen);
            ImGui::EndMainMenuBar();
        }

        if (show_splitscreen) {
            if (ImGui::Begin("Splitscreen", &show_splitscreen, ImGuiWindowFlags_MenuBar))
                RealContext();
            ImGui::End();
        }
    }

    void ProfileContext(int index) {
        char buffer[1024];
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();
        auto p_profile = CGameManager::get_profile(index);
        const char* items[] = {"Controller 1", "Controller 2", "Controller 3", "Controller 4", "NONE"};

        if (p_profile == nullptr || p_setting == nullptr)
            return;

        ImGui::PushItemWidth(200);
        String::convert(buffer, p_profile->name, 1024);
        if (ImGui::InputText("Name", buffer, sizeof(buffer)))
            String::convert(p_profile->name, buffer, 1024);
        ImGui::PopItemWidth();

        ImGui::BeginDisabled(!index && p_setting->b_player0_use_km);

        // Check for controller binding completion
        if (s_binding_player == index) {
            int detected = DetectActiveController();
            if (detected >= 0) {
                p_profile->controller_index = detected;
                s_binding_player = -1;
            }
        }

        if (s_binding_player == index) {
            // Show binding prompt
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press any button on controller...");
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                s_binding_player = -1;
            }
        } else {
            // Show dropdown and bind button
            ImGui::PushItemWidth(200);
            ImGui::Combo("Input", &p_profile->controller_index, items, IM_ARRAYSIZE(items));
            ImGui::PopItemWidth();
            ImGui::SameLine();
            sprintf(buffer, "Bind##ctrl%d", index);
            if (ImGui::Button(buffer) && s_binding_player < 0) {
                s_binding_player = index;
            }
        }

        ImGui::EndDisabled();

        if (ImGui::Button("Apply Profile")) {
            LOG_INFO("Apply Profile clicked for player {}", index);
            __int64 xuid;
            auto p_mng = GameManager();
            auto p_engine = GameEngine();
            if (MCC::IsInGame() && p_mng && (xuid = CGameManager::get_xuid(0))) {
                auto src_profile = p_mng->ppOriginal.get_player_profile(p_mng, xuid);
                auto src_mapping = p_mng->ppOriginal.retrive_gamepad_mapping(p_mng, xuid);
                LOG_DEBUG("Apply Profile: src_profile={:p}, src_mapping={:p}", (void*)src_profile, (void*)src_mapping);
                if (src_profile && src_mapping) {
                    memcpy(&p_profile->profile, src_profile, sizeof(CUserProfile));
                    memcpy(&p_profile->mapping, src_mapping, sizeof(CGamepadMapping));
                    LOG_INFO("Apply Profile: Copied profile and mapping to player {}", index);
                    if (p_engine)
                        p_engine->load_setting();
                } else {
                    LOG_ERROR("Apply Profile: Source profile or mapping is null!");
                }
            } else {
                LOG_WARNING("Apply Profile: Not in game or GameManager unavailable");
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Use this in game!!!");
        if (ImGui::Button("Save Profile")) {
            MCC::Settings::Profile::CaptureFromRuntime();
            MCC::Settings::Profile::Save();
        }

        bool is_disabled = (!index && !p_setting->b_override_profile) || (index && p_setting->b_use_player0_profile);

        if (ImGui::CollapsingHeader("Gamepad Mapping")) {
            ImGui::Indent();
            ImGui::BeginDisabled(is_disabled);
            p_profile->mapping.ImGuiContext();
            ImGui::EndDisabled();
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Profile")) {
            ImGui::Indent();
            ImGui::BeginDisabled(is_disabled);
            // seeded before the first edit, which a profile edited in only, say, its look deadzones
            // would otherwise lose to player 1's settings at match start (a zero FOV marks it unseeded)
            if (!is_disabled) CGameManager::seed_profile(p_profile->profile);
            p_profile->profile.ImGuiContext();
            ImGui::EndDisabled();
            ImGui::Unindent();
        }
    }

    void RealContext() {
        bool dirty = false;
        char buffer[10];
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();

        if (ImGui::BeginMenuBar()) {
            // ImGui::MenuItem(p_setting->b_override ? "Disable" : "Enable", nullptr, &p_setting->b_override);
            dirty |= ImGui::MenuItem(
                p_setting->b_override ? "Disable" : "Enable",
                nullptr,
                &p_setting->b_override
            );
            if (ImGui::BeginMenu("Options")) {
                // ImGui::MenuItem("Use player1's profile", nullptr, &p_setting->b_use_player0_profile);
                dirty |= ImGui::MenuItem(
                    "Use player1's profile", 
                    nullptr, 
                    &p_setting->b_use_player0_profile
                );
                // ImGui::MenuItem("Enable K/M for player1", nullptr, &p_setting->b_player0_use_km);
                dirty |= ImGui::MenuItem(
                    "Enable K/M for player1", 
                    nullptr, 
                    &p_setting->b_player0_use_km
                );
                // ImGui::MenuItem("Override profile", nullptr, &p_setting->b_override_profile);
                dirty |= ImGui::MenuItem(
                    "Override profile", 
                    nullptr, 
                    &p_setting->b_override_profile
                );
                bool left_right = LeftRight::Chosen();
                if (ImGui::MenuItem("Side-by-side split (2-3 players)", nullptr, &left_right))
                    LeftRight::Choose(left_right);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Left / Right instead of Top / Bottom: 2 players get a full-height half each; with 3, "
                                      "player 1 has the left half and players 2 and 3 share the right. Everyone's "
                                      "setting, also in each player's menu (MY HUD > SPLIT). Halo CE and Halo 2 change at "
                                      "the next mission start, in Classic graphics (Back to Anniversary shows their "
                                      "stacked or quarter views).");
                bool hot_join = g_hot_join.Get();
                if (ImGui::MenuItem("Hot join (experimental)", nullptr, &hot_join, !HotJoinLocked()))
                    ChooseHotJoin(hot_join);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Players join in the middle of a mission: a controller nobody is playing with "
                                      "presses A to join (they come in beside a teammate). In Halo CE the last player "
                                      "can also leave, with B in the Players window; in the other games players leave "
                                      "at MCC's menus. Halo CE and Halo 2 take it as a mission starts, so in their "
                                      "games it's changed at MCC's menus.");
                bool anniversary = AnniversaryQuadChosen();
                if (ImGui::MenuItem("Anniversary graphics with 3-4 players (CE, H2; experimental)", nullptr, &anniversary))
                    ChooseAnniversaryQuad(anniversary);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Starts Halo CE and Halo 2 missions with 3 or 4 players in Anniversary graphics instead "
                                      "of Classic; Back switches between them either way. The games' renderers only draw two "
                                      "views at a time, so they alternate: players 1 and 2 on one frame, 3 and 4 on the next "
                                      "(each view updates at half the frame rate). Changes at the next mission start; side by "
                                      "side starts in Classic. Everyone's setting, also in each player's menu (MY HUD > ANNIV 3-4P).");
                ImGui::EndMenu();
            }
#pragma region player count
            ImGui::PushItemWidth(200);
            // int count = p_setting->player_count;
            // if (ImGui::InputInt("Players", &count) && count >= 1 && count <=4) {
            //     p_setting->player_count = count;
            // }
            int count = p_setting->player_count;
            if (ImGui::InputInt("Players", &count) && count >= 1 && count <= 4) {
                p_setting->player_count = count;
                dirty = true;
            }
            ImGui::PopItemWidth();
            ImGui::EndMenuBar();
#pragma endregion
        }

        if (ImGui::BeginTabBar("Players")) {
            for (int i = 0; i < p_setting->player_count; ++i) {
                sprintf(buffer, "Player %d", i + 1);
                if (ImGui::BeginTabItem(buffer)) {
                    ProfileContext(i);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        if (dirty) {
            MCC::Settings::Splitscreen::CaptureFromRuntime();
            // MCC::Settings::Profile::CaptureFromRuntime();
            MCC::Settings::Splitscreen::Save();
            // MCC::Settings::Profile::Save();
        }
    }
}