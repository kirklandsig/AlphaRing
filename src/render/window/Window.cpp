#include <tchar.h>
#include "Window.h"

#include "common.h"

#include "global/Global.h"
#include "input/Input.h"
#include "input/MenuConfig.h"

#include "imgui.h"

#include "../D3d11/D3d11.h"

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace AlphaRing::Render::Window {
    WNDPROC oldWndProc = nullptr;

    //todo: WM_IME_COMPOSITION Support
    static LRESULT dWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_DEVICECHANGE)
            AlphaRing::Input::RequestPadRescan();

        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;


        // Menu trigger: toggle on the initial keydown only (bit 30 = key was
        // already down, i.e. autorepeat) and consume both keydown and keyup so
        // a bind like SPACE or A-Z never reaches the game. Exception: while
        // typing in an overlay text field the key must type, not toggle.
        if ((uMsg == WM_KEYDOWN || uMsg == WM_KEYUP) && static_cast<int>(wParam) == g_menuConfig.keyboardVKey) {
            bool typing = AlphaRing::Global::Global()->show_imgui && ImGui::GetIO().WantTextInput;
            if (!typing) {
                if (uMsg == WM_KEYDOWN && !(lParam & (1 << 30)))
                    AlphaRing::Global::Global()->show_imgui = !AlphaRing::Global::Global()->show_imgui;
                return 0;
            }
        }

        // Only swallow the input ImGui is actually consuming. Swallowing every
        // message (WM_PAINT, WM_ACTIVATE, ...) while the cursor hovers the
        // overlay starves the game's message pump: WantCapture* only refreshes
        // on the next ImGui frame, which never comes, so the game hangs (seen
        // under Proton, where the cursor starts at 0,0 over the menu bar).
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
