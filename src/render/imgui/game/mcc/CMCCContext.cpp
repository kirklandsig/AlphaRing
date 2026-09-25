#include "CMCCContext.h"

#include "global/Global.h"

#include "input/MenuConfig.h"
#include "mcc/mcc.h"
#include "mcc/CGameManager.h"
#include "mcc/network/Network.h"
#include "mcc/splitscreen/Splitscreen.h"
#include "mcc/spawn/Spawn.h"
#include "mcc/hud/Hud.h"
#include "mcc/module/Module.h"

static auto msg_about = R"(
        Alpha Ring
                Made by WinterSquire
)";

static bool show_home = true; // shown the first time the overlay opens
static bool show_about = false;

// What the overlay does and how to drive it; opens the first time the overlay is shown.
static void HomeWindow() {
    auto* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(620, 0), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Alpha Ring - Home", &show_home, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    auto p_setting = AlphaRing::Global::MCC::Splitscreen();
    ImGui::SeparatorText("Split-screen");
    if (p_setting->b_override)
        ImGui::Text("On: %d local players (Splitscreen menu to change players and controllers)",
                    CGameManager::active_player_count());
    else
        ImGui::TextWrapped("Off. Open Splitscreen in the menu bar, tick Override and pick 2-4 players.");

    ImGui::SeparatorText("Spawn menu");
    if (!MCC::IsInGame())
        ImGui::TextDisabled("Start a campaign mission of Halo CE, Halo 2, Halo 3 or ODST.");
    else if (MCC::Spawn::Catalog::CurrentGame() >= 0)
        ImGui::TextColored(ImVec4(0.38f, 0.84f, 0.44f, 1.0f), "Available in this game.");
    else
        ImGui::TextDisabled("Not available in this game yet (Halo CE, Halo 2, Halo 3 and ODST are).");
    ImGui::TextWrapped("In game, each player presses %s to open their own spawn menu in their part of the "
                       "screen. Their Spartan holds still while it's open.",
                       g_menuConfig.spawnMenuMask == XINPUT_GAMEPAD_DPAD_DOWN ? "D-pad Down" : "the spawn button");
    ImGui::BulletText("LB / RB: vehicles, weapons, equipment, characters");
    ImGui::BulletText("Up / Down: choose;  A: spawn in front of you;  B: close");
    ImGui::BulletText("Characters: Left / Right picks their side (their own, ally, enemy)");
    ImGui::BulletText("Y: refresh the list after reaching a new area");
    ImGui::TextDisabled("The Spawn window in the menu bar does the same with the mouse.");

    ImGui::SeparatorText("This overlay");
    ImGui::TextWrapped("F4 or Start + Back shows and hides it. With a controller, the right stick moves the "
                       "cursor and RB clicks.");

    ImGui::Spacing();
    if (ImGui::Button("Close overlay")) AlphaRing::Global::Global()->show_imgui = false;
    ImGui::End();
}

CMCCContext CMCCContext::instance;
ICContext* g_pMCCContext = &CMCCContext::instance;

void CMCCContext::render() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Alpha Ring")) {
            if (ImGui::BeginMenu("Options")) {
                {ImGui::MenuItem("On Menu", nullptr, false, false);
                    ImGui::MenuItem("Pause Game", nullptr, &AlphaRing::Global::Global()->pause_game_on_menu_shown);
                    ImGui::MenuItem("Disable Input", nullptr, &AlphaRing::Global::Global()->disable_input_on_menu_shown);
                    ImGui::MenuItem("Show Mouse", nullptr, &AlphaRing::Global::Global()->show_imgui_mouse);
                }
                {ImGui::MenuItem("Render", nullptr, false, false);
                    ImGui::MenuItem("Wireframe", nullptr, &AlphaRing::Global::Global()->wireframe);
                }
                ImGui::EndMenu();
            }

            ImGui::MenuItem("Home", nullptr, &show_home);
            ImGui::MenuItem("About", nullptr, &show_about);
            if (ImGui::MenuItem("Close", nullptr)) AlphaRing::Global::Global()->show_imgui = false;
//            if (ImGui::MenuItem("Exit", nullptr)) ExitProcess(1);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::EndMainMenuBar();
    }

    MCC::Module::ImGuiContext();

    MCC::Splitscreen::ImGuiContext();

    MCC::Spawn::ImGuiContext();

    MCC::Hud::ImGuiContext();

    MCC::Network::ImGuiContext();

    if (ImGui::BeginMainMenuBar()) {
        ImGui::Separator();
        ImGui::EndMainMenuBar();
    }

    if (show_home)
        HomeWindow();

    if (show_about) {
        ImGui::Begin("About", &show_about);
        ImGui::Text(msg_about);
        ImGui::End();
    }
}
