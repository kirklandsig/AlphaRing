#include "imgui_render.h"

#include "../mcc/player_manager.h"
#include "../mcc/mcc_manager.h"

#include <imgui.h>
#include <libmcc/libmcc.h>

#include <Windows.h>
#include <Xinput.h>

using namespace libmcc;

// Defined in mcc_impl_1.cpp: lazily loads xinput and returns XInputGetState.
typedef DWORD (*t_XInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
t_XInputGetState get_xinput_get_state_function();

namespace {
	// Previous-frame button states for rising-edge detection.
	bool g_prev_pad_a[XUSER_MAX_COUNT] = {};
	bool g_prev_pad_b[XUSER_MAX_COUNT] = {};
	bool g_prev_space = false;
	bool g_prev_backspace = false;

	// True from the moment a game module loads until we return to the menu. Unlike
	// the transient _game_globals_state_launch, the module stays set for the whole
	// match and is cleared only on exit (see mcc_impl_2.cpp), so joins stay locked
	// for the entire match and the overlay can hide during play.
	bool in_match() {
		return mcc_manager()->get_game_module() != k_module_none;
	}

	void poll_input() {
		auto pm = player_manager();

		// Gamepads: A joins, B leaves (rising edge only).
		if (auto xinput = get_xinput_get_state_function()) {
			// Auto-assign P1 to the primary (lowest-connected) controller, so the
			// menu host never has to press A to join — which would also activate the
			// focused Halo menu button. Sticky: re-applies whenever the lobby is
			// empty and a pad is present, since MCC gives menu control to that pad.
			if (pm->get_local_player_count() == 0) {
				for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
					XINPUT_STATE st{};
					if (xinput(i, &st) == ERROR_SUCCESS) {
						pm->join(static_cast<e_player_input_device>(_player_input_device_gamepad_0 + i));
						break;
					}
				}
			}

			for (DWORD i = 0; i < XUSER_MAX_COUNT && i < (DWORD)k_local_player_count; ++i) {
				XINPUT_STATE st{};
				bool connected = xinput(i, &st) == ERROR_SUCCESS;
				bool a = connected && (st.Gamepad.wButtons & XINPUT_GAMEPAD_A);
				bool b = connected && (st.Gamepad.wButtons & XINPUT_GAMEPAD_B);

				auto device = static_cast<e_player_input_device>(_player_input_device_gamepad_0 + i);

				if (a && !g_prev_pad_a[i]) {
					pm->join(device);
				}

				if (b && !g_prev_pad_b[i]) {
					int slot = pm->find_slot(device);
					if (slot != k_player_input_device_none) {
						pm->leave(slot);
					}
				}

				g_prev_pad_a[i] = a;
				g_prev_pad_b[i] = b;
			}
		}

		// Keyboard/mouse: Space joins, Backspace leaves. Suppressed while ImGui is
		// capturing typed text (e.g. the GameEngine command box).
		bool kb = !ImGui::GetIO().WantCaptureKeyboard;
		bool space = kb && (GetAsyncKeyState(VK_SPACE) & 0x8000);
		bool backspace = kb && (GetAsyncKeyState(VK_BACK) & 0x8000);

		if (space && !g_prev_space) {
			pm->join(_player_input_device_km);
		}

		if (backspace && !g_prev_backspace) {
			int slot = pm->find_slot(_player_input_device_km);
			if (slot != k_player_input_device_none) {
				pm->leave(slot);
			}
		}

		g_prev_space = space;
		g_prev_backspace = backspace;
	}
}

void c_imgui_render::join() {
	// Lock joins and hide the overlay for the duration of a match; it returns when
	// we are back in the menus.
	if (in_match()) {
		return;
	}

	poll_input();

	auto pm = player_manager();
	int count = pm->get_local_player_count();

	auto& io = ImGui::GetIO();

	// Top-right, passthrough overlay (never captures input / blocks the game).
	const float margin = 12.0f;
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - margin, margin), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.55f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin("##splitscreen_join", nullptr, flags)) {
		ImGui::TextUnformatted("Splitscreen");
		ImGui::Separator();

		for (int slot = 0; slot < k_local_player_count; ++slot) {
			if (slot < count) {
				auto device = pm->device_at(slot);
				ImGui::Text("P%d  %s", slot + 1, k_player_input_device_names[device]);
				ImGui::SameLine();

				// The auto-assigned controller host (slot 0) is sticky, so it has no
				// leave prompt; everyone else can drop out.
				bool is_pad_host = (slot == 0 && device != _player_input_device_km);
				ImGui::TextDisabled(is_pad_host ? "(host)" : "(B / Backspace)");
			} else if (slot == count) {
				ImGui::Text("P%d  Press A / Space to Join", slot + 1);
			} else {
				ImGui::TextDisabled("P%d  --", slot + 1);
			}
		}
	}
	ImGui::End();
}
