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

        auto& io = ImGui::GetIO();

        if (io.WantCaptureMouse)        
            if(AlphaRing::Global::Global()->show_imgui)
                return true;        

        return CallWindowProc(oldWndProc, hWnd, uMsg, wParam, lParam);
    }

    bool Initialize() {
        oldWndProc = (WNDPROC)SetWindowLongPtr(Graphics()->hwnd, GWLP_WNDPROC, (LONG_PTR)dWndProc);

        return oldWndProc != nullptr;
    }
}
