#include "ImGui.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <algorithm>

#include "input/Input.h"
#include "global/Global.h"
#include "filesystem/Filesystem.h"

#include "../D3d11/D3d11.h"
#include "./game/mcc/CMCCContext.h"
#include "./game/halo3/CHalo3Context.h"

#include "mcc/mcc.h"
#include "mcc/CGameGlobal.h"
#include "mcc/spawn/Spawn.h"

#include <initializer_list>

static ICContext* pages[7] {
        nullptr,
        nullptr,
        g_pHalo3Context,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
};

namespace AlphaRing::Render::ImGui {
    static ImFont* s_menu_font;

    ImFont* MenuFont() { return s_menu_font; }

    static const char* FirstExisting(std::initializer_list<const char*> paths) {
        for (auto path : paths)
            if (AlphaRing::Filesystem::Exist(path)) return path;
        return nullptr;
    }

    // Dark slate with Halo-HUD blue accents.
    static void ApplyTheme(float scale) {
        auto& style = ::ImGui::GetStyle();
        ::ImGui::StyleColorsDark(&style);

        style.WindowRounding = 8.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 5.0f;
        style.TabRounding = 5.0f;
        style.WindowPadding = ImVec2(12, 10);
        style.FramePadding = ImVec2(10, 5);
        style.ItemSpacing = ImVec2(9, 7);
        style.ItemInnerSpacing = ImVec2(7, 5);
        style.ScrollbarSize = 14.0f;
        style.WindowTitleAlign = ImVec2(0.02f, 0.5f);

        const ImVec4 accent(0.25f, 0.66f, 0.96f, 1.0f);
        auto with_alpha = [](ImVec4 c, float a) { c.w = a; return c; };
        auto* colors = style.Colors;
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.96f, 1.0f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.52f, 0.58f, 0.65f, 1.0f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.08f, 0.11f, 0.95f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.10f, 0.14f, 0.60f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.09f, 0.12f, 0.97f);
        colors[ImGuiCol_Border] = with_alpha(accent, 0.25f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.17f, 0.22f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.24f, 0.31f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.30f, 0.40f, 1.0f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.09f, 0.12f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.17f, 0.26f, 1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.09f, 0.12f, 0.80f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.09f, 0.12f, 0.97f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab] = with_alpha(accent, 0.35f);
        colors[ImGuiCol_ScrollbarGrabHovered] = with_alpha(accent, 0.55f);
        colors[ImGuiCol_ScrollbarGrabActive] = with_alpha(accent, 0.75f);
        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = with_alpha(accent, 0.80f);
        colors[ImGuiCol_SliderGrabActive] = accent;
        colors[ImGuiCol_Button] = ImVec4(0.14f, 0.27f, 0.41f, 0.90f);
        colors[ImGuiCol_ButtonHovered] = with_alpha(accent, 0.75f);
        colors[ImGuiCol_ButtonActive] = accent;
        colors[ImGuiCol_Header] = with_alpha(accent, 0.30f);
        colors[ImGuiCol_HeaderHovered] = with_alpha(accent, 0.50f);
        colors[ImGuiCol_HeaderActive] = with_alpha(accent, 0.70f);
        colors[ImGuiCol_Separator] = with_alpha(accent, 0.20f);
        colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.20f, 1.0f);
        colors[ImGuiCol_TabHovered] = with_alpha(accent, 0.60f);
        colors[ImGuiCol_TabActive] = ImVec4(0.16f, 0.36f, 0.56f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.09f, 0.12f, 0.16f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.26f, 0.40f, 1.0f);
        colors[ImGuiCol_NavHighlight] = accent;

        style.ScaleAllSizes(scale);
    }

    bool Initialize() {
        ::ImGui::CreateContext();
        ImGui_ImplWin32_Init(Graphics()->hwnd);
        ImGui_ImplDX11_Init(Graphics()->pDevice, Graphics()->pContext);
        ImGuiIO &io = ::ImGui::GetIO();

        // config
        io.MouseDrawCursor = true;
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        // ini
        io.IniFilename = "./alpha_ring/imgui.ini";
        ::ImGui::LoadIniSettingsFromDisk("../../../alpha_ring/imgui.ini");

        // Windows DPI scaling, or the screen's size relative to 1080p where DPI is always 96
        // (Proton): otherwise the overlay is tiny on a 4K TV.
        const float scale = (std::max)(GetDpiForWindow(Graphics()->hwnd) / 96.0f,
                                     (std::min)(GetSystemMetrics(SM_CXSCREEN) / 1920.0f, GetSystemMetrics(SM_CYSCREEN) / 1080.0f));

        // Microsoft YaHei covers Chinese; Proton prefixes ship it as .ttf, and Arial/Tahoma.
        io.Fonts->Clear();
        if (auto path = FirstExisting({R"(C:\Windows\Fonts\msyh.ttc)", R"(C:\Windows\Fonts\msyh.ttf)"})) {
            io.Fonts->AddFontFromFileTTF(path, 18.0f * scale, nullptr, io.Fonts->GetGlyphRangesChineseFull());
        } else if (auto path = FirstExisting({R"(C:\Windows\Fonts\segoeui.ttf)", R"(C:\Windows\Fonts\arial.ttf)",
                                              R"(C:\Windows\Fonts\tahoma.ttf)"})) {
            io.Fonts->AddFontFromFileTTF(path, 18.0f * scale);
        } else {
            ImFontConfig config;
            config.SizePixels = 16.0f * scale;
            io.Fonts->AddFontDefault(&config);
        }

        // Controller menus are read from the couch: a big bold face, drawn scaled down.
        if (auto path = FirstExisting({R"(C:\Windows\Fonts\bahnschrift.ttf)", R"(C:\Windows\Fonts\segoeuib.ttf)",
                                       R"(C:\Windows\Fonts\arialbd.ttf)", R"(C:\Windows\Fonts\tahomabd.ttf)"}))
            s_menu_font = io.Fonts->AddFontFromFileTTF(path, 34.0f * scale);
        if (s_menu_font == nullptr)
            s_menu_font = io.Fonts->Fonts[0];

        ApplyTheme(scale);

        // Build the font atlas now: it would otherwise be built on the first frame, which is a
        // player opening a spawn menu mid-mission.
        ImGui_ImplDX11_CreateDeviceObjects();

        return true;
    }

    static void RenderOverlay();

    void Render() {
        // Without the overlay, ImGui runs only to draw the players' spawn menus, and not at all
        // otherwise, so it can't capture input meant for the game's own menus.
        bool show = AlphaRing::Global::Global()->show_imgui;
        // Hidden, the overlay isn't fed input (Window.cpp), so it wouldn't see a key or button
        // held at that moment being released: let it go of everything now.
        static bool was_shown;
        if (was_shown && !show) ::ImGui::GetIO().ClearInputKeys();
        was_shown = show;
        if (!show && !MCC::Spawn::AnyPlayerMenuOpen()) {
            AlphaRing::Input::Update();
            return;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ::ImGui::NewFrame();

        AlphaRing::Input::Update();

        if (show)
            RenderOverlay();
        else
            ::ImGui::SetMouseCursor(ImGuiMouseCursor_None);

        MCC::Spawn::RenderPlayerMenus();

        ::ImGui::Render();
        Graphics()->SetRenderTargetView();
        ImGui_ImplDX11_RenderDrawData(::ImGui::GetDrawData());
    }

    static void RenderOverlay() {
        bool inGame = MCC::IsInGame();
        auto pGameGlobal = GameGlobal();

        if (!AlphaRing::Global::Global()->show_imgui_mouse)
            ::ImGui::SetMouseCursor(ImGuiMouseCursor_None);

        g_pMCCContext->render();


        if (inGame && pGameGlobal != nullptr) {
            // Bounds check: pages array has 7 elements (indices 0-6)
            if (pGameGlobal->current_game > 0 && pGameGlobal->current_game < 7)
            {
                auto context = pages[pGameGlobal->current_game];
                if (context != nullptr)
                    context->render();
            }
        }

        if (::ImGui::BeginMainMenuBar()) {
            if (inGame)
                ::ImGui::Separator();
            ::ImGui::Text("%.1f fps", ::ImGui::GetIO().Framerate);
            ::ImGui::EndMainMenuBar();
        }
    }
}