// Per-player spawn menus: a player presses the spawn button (D-pad Down by default) and
// drives a small menu in their own split-screen view with their controller while their
// Spartan stands still. Input arrives on the game's input thread (HandlePlayerInput), the
// menus act and draw on the render thread (RenderPlayerMenus).

#define NOMINMAX // std::min/std::max
#include "Spawn.h"

#include "global/Global.h"
#include "input/MenuConfig.h"
#include "render/imgui/ImGui.h"

#include "imgui.h"

#include <algorithm>
#include <mutex>

namespace MCC::Spawn {
    namespace {
        constexpr const char* kCategoryTitles[kCategoryCount] = {"VEHICLES", "WEAPONS", "EQUIPMENT", "CHARACTERS"};
        constexpr const char* kTeamNames[kTeamCount] = {"Their own side", "Ally", "Enemy"};

        struct Menu {
            bool open = false;
            bool refresh = false;       // ask for a fresh catalog on the next frame
            int category = Vehicles;
            int selected[kCategoryCount] = {};
            int team = TeamEnemy;
            XINPUT_GAMEPAD pad = {};    // latest pad (input thread)
            WORD seen = 0;              // buttons at the previous poll (input thread)
            WORD handled = 0;           // buttons already acted on (render thread)
            int repeat_dir = 0;         // held up/down direction and its next repeat time
            ULONGLONG repeat_at = 0;
            int first_row = 0;          // scroll position
            bool held = false;          // closed, but the pad is kept until its buttons are released
        };

        std::mutex s_mutex;
        Menu s_menus[kMaxPlayers];

        // Screen area of `player`'s view: stacked halves for two players, quarters for three or four.
        ImVec4 ViewRect(int player, int count, ImVec2 display) {
            if (count <= 1) return {0, 0, display.x, display.y};
            if (count == 2) return {0, display.y * 0.5f * player, display.x, display.y * 0.5f};
            float w = display.x * 0.5f, h = display.y * 0.5f;
            return {w * (player % 2), h * (player / 2), w, h};
        }

        constexpr ImU32 kPlayerColors[kMaxPlayers] = {
            IM_COL32(64, 170, 255, 255), IM_COL32(255, 92, 84, 255), IM_COL32(96, 214, 112, 255), IM_COL32(255, 196, 64, 255)};
        constexpr ImU32 kTeamColors[kTeamCount] = {IM_COL32(190, 200, 210, 255), IM_COL32(96, 214, 112, 255), IM_COL32(255, 92, 84, 255)};

        ImU32 WithAlpha(ImU32 color, int alpha) { return (color & ~IM_COL32_A_MASK) | ((ImU32)alpha << IM_COL32_A_SHIFT); }

        // Draws text and returns its width.
        float Text(ImDrawList* dl, ImFont* font, float size, ImVec2 pos, ImU32 color, const char* text) {
            dl->AddText(font, size, pos, color, text);
            return font->CalcTextSizeA(size, FLT_MAX, 0, text).x;
        }

        float ButtonWidth(ImFont* font, float size, const char* button) {
            return std::max(size * 1.1f, font->CalcTextSizeA(size * 0.8f, FLT_MAX, 0, button).x + size * 0.6f);
        }

        // A controller button glyph ("A" in a green disc, "LB" in a pill) followed by a label;
        // returns the total width.
        float ButtonHint(ImDrawList* dl, ImFont* font, float size, ImVec2 pos, const char* button, const char* label) {
            static const struct { const char* name; ImU32 color; } kButtons[] = {
                {"A", IM_COL32(60, 176, 67, 255)}, {"B", IM_COL32(224, 60, 49, 255)},
                {"X", IM_COL32(31, 117, 254, 255)}, {"Y", IM_COL32(240, 200, 8, 255)}};
            ImU32 color = IM_COL32(90, 98, 110, 255);
            for (auto& b : kButtons) if (strcmp(b.name, button) == 0) color = b.color;

            ImVec2 text = font->CalcTextSizeA(size * 0.8f, FLT_MAX, 0, button);
            float h = size * 1.1f, w = ButtonWidth(font, size, button);
            dl->AddRectFilled(pos, {pos.x + w, pos.y + h}, color, h * 0.5f);
            dl->AddText(font, size * 0.8f, {pos.x + (w - text.x) * 0.5f, pos.y + (h - text.y) * 0.5f}, IM_COL32_WHITE, button);
            float x = pos.x + w + size * 0.3f;
            if (label) x += Text(dl, font, size, {x, pos.y + (h - size) * 0.5f}, IM_COL32(215, 222, 230, 255), label);
            return x - pos.x;
        }

        // `text` cut to `width` with an ellipsis.
        std::string Fit(ImFont* font, float size, const std::string& text, float width) {
            if (font->CalcTextSizeA(size, FLT_MAX, 0, text.c_str()).x <= width) return text;
            const char* end = nullptr;
            float room = width - font->CalcTextSizeA(size, FLT_MAX, 0, "...").x;
            font->CalcTextSizeA(size, room, 0, text.c_str(), nullptr, &end);
            return std::string(text.c_str(), end) + "...";
        }

        struct Snapshot { int category, selected, team, first_row; };

        void Draw(int player, const Snapshot& m, int game, ImVec4 view) {
            ImFont* font = AlphaRing::Render::ImGui::MenuFont();
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            ImU32 accent = kPlayerColors[player];

            // sized for a quarter of a 1080p screen, scaled with the view
            float scale = std::clamp(view.z / 960.0f * 0.5f + view.w / 540.0f * 0.5f, 0.6f, 1.6f); // view: x, y, width, height
            float text = 22.0f * scale, minor = 17.0f * scale, pad = 14.0f * scale;
            float w = std::min(view.z * 0.46f, 470.0f * scale), h = view.w * 0.86f;
            ImVec2 p0 = {view.x + view.z * 0.03f, view.y + view.w * 0.07f}, p1 = {p0.x + w, p0.y + h};

            dl->AddRectFilled(p0, p1, IM_COL32(8, 12, 18, 222), 10.0f * scale);
            dl->AddRect(p0, p1, WithAlpha(accent, 170), 10.0f * scale, 0, 2.0f * scale);
            dl->AddRectFilled(p0, {p1.x, p0.y + 5.0f * scale}, accent, 10.0f * scale, ImDrawFlags_RoundCornersTop);

            float x = p0.x + pad, y = p0.y + pad;
            char title[48];
            snprintf(title, sizeof(title), "PLAYER %d  SPAWN", player + 1);
            Text(dl, font, minor, {x, y}, accent, title);
            y += minor + pad * 0.6f;

            // category: LB  < NAME >  RB, with a dot per category
            float lb = ButtonHint(dl, font, minor, {x, y + (text - minor) * 0.3f}, "LB", nullptr);
            float rb = ButtonWidth(font, minor, "RB");
            ButtonHint(dl, font, minor, {p1.x - pad - rb, y + (text - minor) * 0.3f}, "RB", nullptr);
            const char* name = kCategoryTitles[m.category];
            float name_w = font->CalcTextSizeA(text, FLT_MAX, 0, name).x;
            Text(dl, font, text, {x + lb + (w - 2 * pad - lb - rb - name_w) * 0.5f, y}, IM_COL32_WHITE, name);
            y += text + pad * 0.3f;
            float dots_x = p0.x + w * 0.5f - (kCategoryCount - 1) * 6.0f * scale;
            for (int c = 0; c < kCategoryCount; ++c)
                dl->AddCircleFilled({dots_x + c * 12.0f * scale, y}, 3.0f * scale,
                                    c == m.category ? accent : IM_COL32(90, 98, 110, 255));
            y += pad;

            if (m.category == Characters) {
                float sx = x + Text(dl, font, minor, {x, y}, IM_COL32(150, 160, 172, 255), "SIDE  ");
                sx += Text(dl, font, minor, {sx, y}, IM_COL32(150, 160, 172, 255), "< ");
                sx += Text(dl, font, minor, {sx, y}, kTeamColors[m.team], kTeamNames[m.team]);
                Text(dl, font, minor, {sx, y}, IM_COL32(150, 160, 172, 255), " >");
                y += minor + pad * 0.6f;
            }

            // footer: status line and button hints
            float hints_y = p1.y - pad - minor * 1.1f;
            float status_y = hints_y - minor - pad * 0.5f;
            float hx = x;
            hx += ButtonHint(dl, font, minor, {hx, hints_y}, "A", "Spawn") + pad;
            hx += ButtonHint(dl, font, minor, {hx, hints_y}, "B", "Close") + pad;
            ButtonHint(dl, font, minor, {hx, hints_y}, "Y", "Refresh");
            auto status = Catalog::Status(player);
            Text(dl, font, minor, {x, status_y}, IM_COL32(170, 180, 190, 255), Fit(font, minor, status, w - 2 * pad).c_str());

            // the list
            float row = text * 1.45f, top = y, bottom = status_y - pad * 0.5f;
            int visible = std::max(1, (int)((bottom - top) / row));
            dl->PushClipRect({p0.x, top}, {p1.x, bottom}, true);
            Catalog::Read(game, (Category)m.category, [&](const std::vector<Item>* items) {
                if (items == nullptr || items->empty()) {
                    Text(dl, font, minor, {x, top + pad * 0.5f}, IM_COL32(150, 160, 172, 255),
                         items == nullptr ? "Loading..." : "Nothing of this kind is loaded here.");
                    return;
                }
                for (int i = m.first_row; i < (int)items->size() && i < m.first_row + visible; ++i) {
                    float ry = top + (i - m.first_row) * row;
                    bool chosen = i == m.selected;
                    if (chosen) {
                        dl->AddRectFilled({p0.x + pad * 0.5f, ry}, {p1.x - pad * 0.5f, ry + row - 2.0f * scale},
                                          WithAlpha(accent, 70), 6.0f * scale);
                        dl->AddRectFilled({p0.x + pad * 0.5f, ry}, {p0.x + pad * 0.5f + 4.0f * scale, ry + row - 2.0f * scale},
                                          accent, 2.0f * scale);
                    }
                    Text(dl, font, text, {x + (chosen ? 6.0f * scale : 0), ry + (row - text) * 0.4f},
                         chosen ? IM_COL32_WHITE : IM_COL32(185, 194, 204, 255),
                         Fit(font, text, (*items)[i].name, w - 3 * pad).c_str());
                }
                if ((int)items->size() > visible) { // scroll bar
                    float track = bottom - top, thumb = std::max(track * visible / items->size(), 12.0f * scale);
                    float at = top + (track - thumb) * m.first_row / std::max(1, (int)items->size() - visible);
                    dl->AddRectFilled({p1.x - pad * 0.45f, at}, {p1.x - pad * 0.2f, at + thumb}, WithAlpha(accent, 150), 3.0f * scale);
                }
            });
            dl->PopClipRect();

            // remember how many rows fit so scrolling keeps the selection visible
            std::lock_guard<std::mutex> lock(s_mutex);
            auto& menu = s_menus[player];
            if (menu.selected[m.category] < menu.first_row) menu.first_row = menu.selected[m.category];
            if (menu.selected[m.category] >= menu.first_row + visible) menu.first_row = menu.selected[m.category] - visible + 1;
        }
    }

    bool HandlePlayerInput(int player, const XINPUT_GAMEPAD& pad) {
        if (player < 0 || player >= kMaxPlayers) return false;

        std::lock_guard<std::mutex> lock(s_mutex);
        auto& m = s_menus[player];
        WORD pressed = pad.wButtons & ~m.seen;
        m.seen = pad.wButtons;

        if (!m.open) {
            // the buttons that closed the menu stay with it until released (B would melee)
            if (m.held && pad.wButtons) return true;
            m.held = false;

            WORD button = g_menuConfig.spawnMenuMask;
            if (button == 0 || (pad.wButtons & button) != button || !(pressed & button)) return false;
            if (AlphaRing::Global::Global()->show_imgui || Catalog::CurrentGame() < 0) return false;
            m.open = true;
            m.refresh = true;
            // the press that opened the menu isn't a menu action, nor the start of a scroll
            m.handled = pad.wButtons;
            m.repeat_dir = pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ? 1 : pad.wButtons & XINPUT_GAMEPAD_DPAD_UP ? -1 : 0;
            m.repeat_at = ~0ull;
        }
        m.pad = pad;
        return true;
    }

    bool AnyPlayerMenuOpen() {
        std::lock_guard<std::mutex> lock(s_mutex);
        return std::any_of(std::begin(s_menus), std::end(s_menus), [](const Menu& m) { return m.open; });
    }

    void RenderPlayerMenus() {
        int game = Catalog::CurrentGame();
        int count = LocalPlayerCount();
        ImVec2 display = ImGui::GetIO().DisplaySize;

        for (int player = 0; player < kMaxPlayers; ++player) {
            Snapshot snapshot;
            bool refresh = false, spawn = false;
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                auto& m = s_menus[player];
                if (!m.open) continue;
                if (game < 0 || player >= count) { m.open = false; m.held = true; continue; }

                WORD pressed = m.pad.wButtons & ~m.handled;
                m.handled = m.pad.wButtons;

                if (pressed & XINPUT_GAMEPAD_B) { m.open = false; m.held = true; continue; }
                if (pressed & XINPUT_GAMEPAD_LEFT_SHOULDER) m.category = (m.category + kCategoryCount - 1) % kCategoryCount;
                if (pressed & XINPUT_GAMEPAD_RIGHT_SHOULDER) m.category = (m.category + 1) % kCategoryCount;
                if (m.category == Characters) {
                    if (pressed & XINPUT_GAMEPAD_DPAD_LEFT) m.team = (m.team + kTeamCount - 1) % kTeamCount;
                    if (pressed & XINPUT_GAMEPAD_DPAD_RIGHT) m.team = (m.team + 1) % kTeamCount;
                }
                refresh = m.refresh || (pressed & XINPUT_GAMEPAD_Y);
                m.refresh = false;
                spawn = pressed & XINPUT_GAMEPAD_A;

                // up/down (D-pad or left stick) move once, then repeat while held
                int dir = (m.pad.wButtons & XINPUT_GAMEPAD_DPAD_UP) || m.pad.sThumbLY > 20000 ? -1
                        : (m.pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) || m.pad.sThumbLY < -20000 ? 1 : 0;
                auto now = GetTickCount64();
                int step = 0;
                if (dir != m.repeat_dir) { step = dir; m.repeat_at = now + 350; }
                else if (dir && now >= m.repeat_at) { step = dir; m.repeat_at = now + 70; }
                m.repeat_dir = dir;

                if (int size = step ? Catalog::Size(game, (Category)m.category) : 0)
                    m.selected[m.category] = (m.selected[m.category] + step + size) % size;
                snapshot = {m.category, m.selected[m.category], m.team, m.first_row};
            }

            Catalog::Refresh(game, refresh); // also picks up a newly loaded map
            Item item;
            if (spawn && Catalog::Get(game, (Category)snapshot.category, snapshot.selected, item))
                Catalog::Spawn((Category)snapshot.category, item, player, (Team)snapshot.team);

            Draw(player, snapshot, game, ViewRect(player, count, display));
        }
    }
}
