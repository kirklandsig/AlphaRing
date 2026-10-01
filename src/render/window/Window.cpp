#include <tchar.h>
#include "Window.h"

#include "common.h"

#include "global/Global.h"
#include "input/Input.h"
#include "input/MenuConfig.h"
#include "render/imgui/game/xbox/CXboxContext.h"

#include "imgui.h"

#include "../D3d11/D3d11.h"

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace AlphaRing::Render::Window {
    WNDPROC oldWndProc = nullptr;

    // A consumed key-down still reaches the game as WM_CHAR: TranslateMessage queued it before this proc ran, and it
    // can even arrive after the key-up. Each consumed press of a menu key that types (SPACE, A-Z) owes one character,
    // swallowed whenever it comes - also after the menu's close animation, when the key's repeats no longer reach the
    // open menu.
    static int s_owed_chars[2] = {}; // the menu key, the debug key

    static int HotkeyIndex(int vk) {
        return vk == g_menuConfig.keyboardVKey ? 0 : vk == g_menuConfig.debugKeyboardVKey ? 1 : -1;
    }

    // A consumed WM_KEYDOWN: a menu key that types owes its character.
    static void Consumed(int vk) {
        int i = HotkeyIndex(vk);
        if (i >= 0 && MapVirtualKey(vk, MAPVK_VK_TO_CHAR) != 0 && s_owed_chars[i] < 32) ++s_owed_chars[i];
    }

    static bool OwedChar(UINT uMsg, LPARAM lParam) {
        if (uMsg != WM_CHAR && uMsg != WM_SYSCHAR) return false;
        UINT scan = (UINT)(lParam >> 16) & 0xFF;
        int keys[2] = {g_menuConfig.keyboardVKey, g_menuConfig.debugKeyboardVKey};
        for (int i = 0; i < 2; ++i) {
            if (s_owed_chars[i] > 0 && scan == MapVirtualKey(keys[i], MAPVK_VK_TO_VSC)) {
                --s_owed_chars[i];
                return true;
            }
        }
        return false;
    }

    //todo: WM_IME_COMPOSITION Support
    static LRESULT dWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_DEVICECHANGE)
            AlphaRing::Input::RequestPadRescan();
        if (uMsg == WM_KILLFOCUS)
            s_owed_chars[0] = s_owed_chars[1] = 0; // characters queued for this window went with its focus
        if (OwedChar(uMsg, lParam))
            return 0;

        bool xboxOpen = g_pXboxContext && g_pXboxContext->isOpen();

        // Intercept keyboard/mouse before ImGui and the game while the Xbox
        // menu is open. The trigger key toggles the menu; everything else is
        // consumed here. Keyboard navigation is handled by GetAsyncKeyState
        // polling in Input::Update(), so no handleInput calls are needed here.
        if (xboxOpen) {
            if (uMsg == WM_KEYDOWN) {
                // Initial press only (lParam bit 30 = autorepeat): holding the
                // key would otherwise reopen the menu once it has closed.
                if (static_cast<int>(wParam) == g_menuConfig.keyboardVKey && !(lParam & (1 << 30))) {
                    g_pXboxContext->close();
                }
                Consumed(static_cast<int>(wParam));
                return 0; // consume all keyboard input
            }
            switch (uMsg) {
                case WM_KEYUP:
                case WM_CHAR:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MOUSEMOVE:
                case WM_MOUSEWHEEL:
                    return 0;
            }
        }

        // Only feed ImGui real WndProc input while one of our menus is actually
        // visible — otherwise ImGui silently queues mouse/keyboard events for the
        // entire play session with nothing ever consuming them (NewFrame() is
        // also gated on menu visibility), which causes a glitch/jump once the
        // menu is finally opened again after a long session.
        bool menuActive = xboxOpen || AlphaRing::Global::Global()->show_imgui;
        if (menuActive && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;

        // Menu keys (only reached when the Xbox menu is closed): consume keydown
        // and keyup alike so a bind like SPACE or A-Z never reaches the game,
        // and open on the initial press only (lParam bit 30 = autorepeat). The
        // debug UI key is toggled by Input::Update(), so here it is only
        // consumed. While typing in a debug UI text field, both keys type.
        if ((uMsg == WM_KEYDOWN || uMsg == WM_KEYUP) &&
            !(AlphaRing::Global::Global()->show_imgui && ImGui::GetIO().WantTextInput)) {
            int vk = static_cast<int>(wParam);
            if (vk == g_menuConfig.keyboardVKey) {
                if (uMsg == WM_KEYDOWN && !(lParam & (1 << 30)) && g_pXboxContext)
                    g_pXboxContext->open();
                if (uMsg == WM_KEYDOWN) Consumed(vk);
                return 0;
            }
            if (vk == g_menuConfig.debugKeyboardVKey) {
                if (uMsg == WM_KEYDOWN) Consumed(vk);
                return 0;
            }
        }

        // Swallow only the input the debug UI is using. Returning true for every
        // message while the cursor is over it also kept WM_PAINT, WM_SIZE,
        // WM_ACTIVATEAPP, WM_CLOSE etc. from the game, and WantCaptureMouse only
        // refreshes on the next ImGui frame, which a game waiting on one of
        // those never draws (seen as a hang under Proton).
        if (AlphaRing::Global::Global()->show_imgui) {
            auto& io = ImGui::GetIO();
            bool mouse_msg = (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST);
            bool key_msg = (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST);
            if ((mouse_msg && io.WantCaptureMouse) || (key_msg && io.WantCaptureKeyboard))
                return 0;
        }

        return CallWindowProc(oldWndProc, hWnd, uMsg, wParam, lParam);
    }

    bool Initialize() {
        oldWndProc = (WNDPROC)SetWindowLongPtr(Graphics()->hwnd, GWLP_WNDPROC, (LONG_PTR)dWndProc);

        return oldWndProc != nullptr;
    }
}
