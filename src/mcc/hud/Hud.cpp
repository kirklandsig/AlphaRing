#include "Hud.h"

#include "common.h"

#include "mcc/CGameGlobal.h"
#include "mcc/mcc.h"
#include "mcc/spawn/Command.h"

#include <offset_halo1.h>
#include <offset_halo2.h>
#include <offset_halo3.h>
#include <offset_halo3odst.h>
#include <offset_haloreach.h>

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace MCC::Hud {
    static PlayerHud s_players[kPlayers];

    PlayerHud& Player(int player) { return s_players[std::clamp(player, 0, kPlayers - 1)]; }

    // --- the games -----------------------------------------------------------------------------

    // How a game's host offsets are measured.
    enum class Units {
        Canvas,  // the HUD's virtual canvas (Halo 3, ODST, Reach): size = float width, height
        Pixels,  // screen pixels / unit scale (Halo 2): size = int16 view rect {t, l, b, r}
        None,    // offsets come from elsewhere (Halo CE: OffsetCE)
    };

    // Per game: where it keeps the player whose HUD is being drawn, how MCC's element ids map to
    // ours (1 motion sensor, 2-5 grenade kinds, 6 shield/health, 7 weapon, 9 crosshair,
    // 11 equipment, 12 messages; Halo 2 has no separate shield element - its shield sits with
    // the motion tracker), the units of its host offsets, and for the HUD area presets the shape
    // of the box the game lays its HUD out in (0: the whole view; Halo CE: a centered 4:3 box)
    // and which side of it each of our elements sits on (-1 left, 1 right, 0 centered).
    struct Game {
        int id;
        __int64 drawing_user;
        bool user_is_short;
        const signed char* elements; // [16], by MCC element id
        Units units;
        __int64 size, unit_scale;
        float area;
        signed char sides[kElementCount]; // motion sensor, shield, weapon, grenades, crosshair, equipment, messages
        bool (*box_is_area)(__int64 module) = nullptr; // whether the HUD box really is `area` right now
    };

    // Halo 2 lays its HUD out in the whole view unless the profiles' "HUD anchor: Centered" is on
    // and our fix for it (the "HUD at screen edges" patch) has been switched off.
    static bool Halo2HudInView(__int64 module) {
        return !*(const bool*)(module + OFFSET_HALO2_PV_HUD_ASPECT_LOCK) ||
               *(const unsigned char*)(module + OFFSET_HALO2_PF_HUD_ASPECT_LOCK) == 0xEB;
    }

    static constexpr signed char kHalo1Elements[16] = {
        -1, MotionSensor, Grenades, -1, -1, -1, Shield, Weapon, -1, Crosshair, -1, -1, Messages, -1, -1, -1};
    static constexpr signed char kHalo2Elements[16] = {
        -1, MotionSensor, Grenades, -1, -1, -1, -1, Weapon, -1, Crosshair, -1, -1, Messages, -1, -1, -1};
    static constexpr signed char kGen3Elements[16] = {
        -1, MotionSensor, Grenades, Grenades, Grenades, Grenades, Shield, Weapon, -1, Crosshair, -1, Equipment, -1, -1, -1, -1};

    static constexpr Game kGames[] = {
        {CGameGlobal::Halo1, OFFSET_HALO1_PV_HUD_DRAWING_PLAYER, true, kHalo1Elements, Units::None, 0, 0,
         4.0f / 3.0f, {-1, 1, -1, -1, 0, 0, -1}},
        {CGameGlobal::Halo2, OFFSET_HALO2_PV_HUD_DRAWING_PLAYER, false, kHalo2Elements, Units::Pixels,
         OFFSET_HALO2_PV_HUD_VIEW_BOUNDS, OFFSET_HALO2_PV_HUD_UNIT_SCALE, 0.0f, {-1, 0, 1, -1, 0, 0, -1}, Halo2HudInView},
        {CGameGlobal::Halo3, OFFSET_HALO3_PV_HUD_DRAWING_PLAYER, false, kGen3Elements, Units::Canvas,
         OFFSET_HALO3_PV_HUD_CANVAS, 0, 0.0f, {-1, 0, 1, -1, 0, -1, 0}},
        {CGameGlobal::Halo3ODST, OFFSET_HALO3ODST_PV_HUD_DRAWING_PLAYER, false, kGen3Elements, Units::Canvas,
         OFFSET_HALO3ODST_PV_HUD_CANVAS, 0, 0.0f, {-1, -1, 1, 1, 0, 1, 0}},
        {CGameGlobal::HaloReach, OFFSET_HALOREACH_PV_HUD_DRAWING_PLAYER, false, kGen3Elements, Units::Canvas,
         OFFSET_HALOREACH_PV_HUD_CANVAS, 0, 0.0f, {-1, 0, 1, 1, 0, -1, 0}},
    };

    static const Game* CurrentGame(__int64& module) {
        auto p_global = GameGlobal();
        if (p_global == nullptr) return nullptr;
        for (auto& game : kGames) {
            if (game.id != p_global->current_game) continue;
            module = MCC::Command::ModuleBase(game.id);
            return module ? &game : nullptr;
        }
        return nullptr;
    }

    bool Supported() {
        __int64 module;
        return MCC::IsInGame() && CurrentGame(module) != nullptr;
    }

    static int ElementOf(const Game& game, int game_element) {
        return game_element >= 0 && game_element < 16 ? game.elements[game_element] : -1;
    }

    // What `player`'s settings do to one element of `game` in a view `aspect` wide per unit of
    // height (0: no area presets); false when nothing.
    static bool Effective(const Game& game, int player, int game_element, float aspect, float* x, float* y, float* scale) {
        int element = ElementOf(game, game_element);
        if (player < 0 || player >= kPlayers || element < 0) return false;
        auto& settings = s_players[player];
        auto& layout = settings.elements[element];
        *x = layout.x;
        *y = layout.y;
        *scale = layout.hidden ? 0.0f : layout.scale * settings.scale;
        // the area preset moves the sides of the game's HUD box to those of a centered box of that
        // shape (no wider than the view)
        if (settings.area > 0 && aspect > 0.0f) {
            float from = game.area > 0.0f ? std::min(game.area, aspect) : aspect;
            *x += game.sides[element] * 0.5f * (std::min(kAreas[settings.area], aspect) - from) / aspect;
        }
        return *x != 0.0f || *y != 0.0f || *scale != 1.0f;
    }

    bool Transform(int game_element, float* dx, float* dy, float* scale) {
        __int64 module;
        auto game = CurrentGame(module);
        if (game == nullptr) return false;

        // the size of the drawing player's view: the HUD's virtual canvas, or pixels (Halo 2);
        // Halo CE takes only sizes from here (its offsets: OffsetCE)
        float width = 0.0f, height = 1.0f;
        if (game->units == Units::Canvas) {
            auto canvas = (const float*)(module + game->size); // width, height
            width = canvas[0];
            height = canvas[1];
        } else if (game->units == Units::Pixels) {
            auto rect = (const short*)(module + game->size); // top, left, bottom, right
            width = rect[3] - rect[1];
            height = rect[2] - rect[0];
        }

        auto user_at = module + game->drawing_user;
        int user = game->user_is_short ? *(short*)user_at : *(int*)user_at;
        float aspect = height > 0.0f && (!game->box_is_area || game->box_is_area(module)) ? width / height : 0.0f;
        float x, y, s;
        if (!Effective(*game, user, game_element, aspect, &x, &y, &s)) return false;
        *scale *= s;
        if (s == 0.0f || game->units == Units::None || height <= 0.0f) return true;
        if (game->units == Units::Pixels) {
            // Halo 2 moves an element by offset x unit scale x its final scale pixels
            float unit = *(const float*)(module + game->unit_scale) * *scale;
            if (unit <= 0.0f) return true;
            width /= unit;
            height /= unit;
        }
        *dx += x * width;
        *dy -= y * height; // our y points up, the screen's down
        return true;
    }

    bool Anchor(int game_element, int* anchor) {
        float dx = 0.0f, dy = 0.0f, scale = 1.0f;
        if (!Transform(game_element, &dx, &dy, &scale)) return false;
        *anchor = 1; // "unchanged": our offsets are relative to where the element already is
        return true;
    }

    bool OffsetCE(int player, int game_element, float view_width, float view_height, float* dx, float* dy) {
        float x, y, s;
        if (view_height <= 0.0f || !Effective(kGames[0], player, game_element, view_width / view_height, &x, &y, &s) ||
            (x == 0.0f && y == 0.0f))
            return false;
        *dx = x * view_width;
        *dy = -y * view_height;
        return true;
    }

    // --- colours ---------------------------------------------------------------------------------

    static float Wrap(float degrees) { return degrees - 360.0f * std::floor(degrees / 360.0f); }

    // Moves a colour's hue toward `hue`, keeping its brightness, saturation and alpha. Greys and
    // whites stay as they are, and so do strong reds: the HUD uses them for enemies and danger.
    static unsigned Recolor(unsigned argb, float hue, float strength) {
        float r = ((argb >> 16) & 0xFF) / 255.0f, g = ((argb >> 8) & 0xFF) / 255.0f, b = (argb & 0xFF) / 255.0f;
        float max = std::max({r, g, b}), min = std::min({r, g, b}), delta = max - min;
        if (max <= 0.0f || delta / max < 0.15f) return argb;

        float h = max == r ? 60.0f * std::fmod((g - b) / delta, 6.0f) : max == g ? 60.0f * ((b - r) / delta + 2.0f)
                                                                                 : 60.0f * ((r - g) / delta + 4.0f);
        h = Wrap(h);
        if ((h < 18.0f || h > 342.0f) && delta / max > 0.45f) return argb;

        float shift = Wrap(hue - h + 180.0f) - 180.0f; // shortest way around
        h = Wrap(h + shift * strength);

        float s = delta / max, v = max, c = v * s, x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f)), m = v - c;
        float rgb[3];
        switch ((int)(h / 60.0f) % 6) {
            case 0: rgb[0] = c; rgb[1] = x; rgb[2] = 0; break;
            case 1: rgb[0] = x; rgb[1] = c; rgb[2] = 0; break;
            case 2: rgb[0] = 0; rgb[1] = c; rgb[2] = x; break;
            case 3: rgb[0] = 0; rgb[1] = x; rgb[2] = c; break;
            case 4: rgb[0] = x; rgb[1] = 0; rgb[2] = c; break;
            default: rgb[0] = c; rgb[1] = 0; rgb[2] = x; break;
        }
        auto byte = [](float f) { return (unsigned)std::lround(std::clamp(f, 0.0f, 1.0f) * 255.0f); };
        return (argb & 0xFF000000u) | byte(rgb[0] + m) << 16 | byte(rgb[1] + m) << 8 | byte(rgb[2] + m);
    }

    unsigned Color(int user, unsigned argb) {
        if (user < 0 || user >= kPlayers) return argb;
        auto& player = s_players[user];
        return player.recolor ? Recolor(argb, player.hue, player.strength) : argb;
    }

    // --- settings file ---------------------------------------------------------------------------

    static const char* kElementKeys[kElementCount] = {"motion_sensor", "shield", "weapon", "grenades", "crosshair",
                                                      "equipment", "messages"};
    static const char* kElementNames[kElementCount] = {"Motion tracker", "Shield / health", "Weapon / ammo",
                                                       "Grenades", "Crosshair", "Equipment", "Messages"};

    // Next to the game exe, like alpha_ring_menu.cfg.
    static std::filesystem::path ConfigPath() {
        char exe[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, exe, MAX_PATH);
        return std::filesystem::path(exe).parent_path() / "alpha_ring_hud.cfg";
    }

    void Save() {
        std::ofstream out(ConfigPath());
        if (!out) return;
        out << "# AlphaRing per-player HUD settings (edited from the overlay's HUD window)\n";
        for (int p = 0; p < kPlayers; ++p) {
            auto& player = s_players[p];
            std::string prefix = "p" + std::to_string(p + 1) + ".";
            out << prefix << "scale=" << player.scale << "\n" << prefix << "area=" << player.area << "\n"
                << prefix << "recolor=" << player.recolor << "\n" << prefix << "hue=" << player.hue << "\n"
                << prefix << "strength=" << player.strength << "\n";
            for (int e = 0; e < kElementCount; ++e) {
                auto& l = player.elements[e];
                std::string key = prefix + kElementKeys[e];
                out << key << ".x=" << l.x << "\n" << key << ".y=" << l.y << "\n"
                    << key << ".scale=" << l.scale << "\n" << key << ".hidden=" << l.hidden << "\n";
            }
        }
    }

    void Load() {
        std::ifstream in(ConfigPath());
        std::string line;
        while (std::getline(in, line)) {
            auto eq = line.find('=');
            if (line.empty() || line[0] == '#' || line[0] != 'p' || eq == std::string::npos) continue;
            int p = line[1] - '1';
            if (p < 0 || p >= kPlayers || line[2] != '.') continue;
            std::string key = line.substr(3, eq - 3);
            float value = std::strtof(line.c_str() + eq + 1, nullptr);
            auto& player = s_players[p];

            if (key == "scale") player.scale = std::clamp(value, 0.25f, 3.0f);
            else if (key == "area") player.area = std::clamp((int)value, 0, kAreaCount - 1);
            else if (key == "recolor") player.recolor = value != 0.0f;
            else if (key == "hue") player.hue = Wrap(value);
            else if (key == "strength") player.strength = std::clamp(value, 0.0f, 1.0f);
            for (int e = 0; e < kElementCount; ++e) {
                std::string prefix = std::string(kElementKeys[e]) + ".";
                if (key.compare(0, prefix.size(), prefix) != 0) continue;
                auto field = key.substr(prefix.size());
                auto& l = player.elements[e];
                if (field == "x") l.x = std::clamp(value, -0.5f, 0.5f);
                else if (field == "y") l.y = std::clamp(value, -0.5f, 0.5f);
                else if (field == "scale") l.scale = std::clamp(value, 0.25f, 3.0f);
                else if (field == "hidden") l.hidden = value != 0.0f;
            }
        }
    }

    // --- menus ------------------------------------------------------------------------------------

    const char* const kAreaNames[kAreaCount] = {"Game default", "Screen edges", "21:9 box", "16:9 box", "4:3 box"};

    unsigned HueColor(float hue) {
        ImVec4 color;
        ImGui::ColorConvertHSVtoRGB(hue / 360.0f, 0.75f, 1.0f, color.x, color.y, color.z);
        color.w = 1.0f;
        return ImGui::ColorConvertFloat4ToU32(color);
    }

    static bool PlayerTab(int p) {
        auto& player = s_players[p];
        bool changed = false;
        auto edited = [&] { changed |= ImGui::IsItemDeactivatedAfterEdit(); };

        ImGui::SeparatorText("Size");
        ImGui::SetNextItemWidth(260);
        ImGui::SliderFloat("Whole HUD", &player.scale, 0.25f, 3.0f, "%.2fx");
        edited();

        // presets by the shape of screen the player looks at, for views wider than their eyes follow
        ImGui::SetNextItemWidth(260);
        changed |= ImGui::Combo("HUD area", &player.area, kAreaNames, kAreaCount);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Lays the HUD out in a box of this shape in the middle of the player's view: pull it\n"
                              "in from the far edges of an ultrawide (21:9) or multi-monitor view to a TV-shaped\n"
                              "(16:9) or classic (4:3) box, or spread Halo CE's HUD (always a 4:3 box) out to the\n"
                              "screen edges.");

        ImGui::SeparatorText("Colour (Halo 3, ODST, Reach)");
        changed |= ImGui::Checkbox("Recolour this player's HUD", &player.recolor);
        ImGui::BeginDisabled(!player.recolor);
        ImGui::ColorButton("##preview", ImGui::ColorConvertU32ToFloat4(HueColor(player.hue)), ImGuiColorEditFlags_NoTooltip, ImVec2(ImGui::GetFrameHeight() * 2, ImGui::GetFrameHeight()));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(260);
        ImGui::SliderFloat("Hue", &player.hue, 0.0f, 360.0f, "%.0f");
        edited();
        ImGui::SetNextItemWidth(260);
        ImGui::SliderFloat("Strength", &player.strength, 0.0f, 1.0f, "%.2f");
        edited();
        ImGui::EndDisabled();
        ImGui::TextDisabled("Whites and enemy reds keep their colour.");

        ImGui::SeparatorText("Elements");
        if (ImGui::BeginTable("elements", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Element");
            ImGui::TableSetupColumn("Left / right");
            ImGui::TableSetupColumn("Down / up");
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Hide");
            ImGui::TableHeadersRow();
            for (int e = 0; e < kElementCount; ++e) {
                auto& l = player.elements[e];
                ImGui::PushID(e);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(kElementNames[e]);
                // the aim point doesn't move with the crosshair's picture, so it can only be resized or hidden
                ImGui::BeginDisabled(e == Crosshair);
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(150);
                ImGui::SliderFloat("##x", &l.x, -0.5f, 0.5f, "%+.2f");
                edited();
                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(150);
                ImGui::SliderFloat("##y", &l.y, -0.5f, 0.5f, "%+.2f");
                edited();
                ImGui::EndDisabled();
                ImGui::TableSetColumnIndex(3);
                ImGui::SetNextItemWidth(130);
                ImGui::SliderFloat("##scale", &l.scale, 0.25f, 3.0f, "%.2fx");
                edited();
                ImGui::TableSetColumnIndex(4);
                changed |= ImGui::Checkbox("##hide", &l.hidden);
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::TextDisabled("Offsets are fractions of the player's view. Messages move in Halo CE and Halo 2, equipment");
        ImGui::TextDisabled("in Halo 3, ODST and Reach; Halo 2's shield moves with its motion tracker.");

        ImGui::Spacing();
        if (ImGui::Button("Reset this player")) { player = PlayerHud(); changed = true; }
        ImGui::SameLine();
        if (ImGui::Button("Copy to all players")) {
            for (auto& other : s_players) other = player;
            changed = true;
        }
        return changed;
    }

    void ImGuiContext() {
        static bool show;

        if (ImGui::BeginMainMenuBar()) {
            ImGui::MenuItem("HUD", nullptr, &show);
            ImGui::EndMainMenuBar();
        }
        if (!show) return;

        ImGui::SetNextWindowSize(ImVec2(760, 0), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("HUD", &show)) {
            ImGui::TextWrapped("Move, resize, hide or recolour each player's HUD - in Halo CE, Halo 2, Halo 3, ODST and "
                               "Halo Reach. Changes show up right away and are saved automatically.");
            if (ImGui::BeginTabBar("players")) {
                for (int p = 0; p < kPlayers; ++p) {
                    char label[16];
                    snprintf(label, sizeof(label), "Player %d", p + 1);
                    if (ImGui::BeginTabItem(label)) {
                        if (PlayerTab(p)) Save();
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}
