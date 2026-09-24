#include "CGameManager.h"

#include "common.h"

#include "./mcc.h"
#include "global/Global.h"
#include "input/Input.h"
#include "input/MenuConfig.h"
#include "mcc/spawn/Spawn.h"

#include <atomic>

void CGameManager::set_vibration(CGameManager *self, DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) {
    CInputDevice* p_device;
    auto p_setting = AlphaRing::Global::MCC::Splitscreen();

    if (!p_setting->b_override) {
        ppOriginal.set_vibration(self, dwUserIndex, pVibration);
        return;
    }

    if (dwUserIndex >= p_setting->player_count)
        return;

    if (pVibration == nullptr || (p_device = get_controller(dwUserIndex)) == nullptr)
        return;

    AlphaRing::Input::SetState(p_device->input_user, pVibration);
}

// Halo 4 campaigns fail when a mission *starts* with players 3-4 (every view stays black),
// yet the engine spawns late joiners next to a teammate, like a controller signing in
// mid-game on the original console. So Halo 4 starts with two players and the rest are
// revealed once the game has been running for a moment. (Halo CE and Halo 2 never pick up
// late joiners, so they - like the other games - start with everyone.)
static constexpr int kDeferredJoinStartPlayers = 2;
static constexpr ULONGLONG kDeferredJoinDelayMs = 3000;
static std::atomic<ULONGLONG> s_running_since; // 0 until the current session first runs

static std::atomic<unsigned> s_load_generation;

unsigned CGameManager::load_generation() { return s_load_generation; }

void CGameManager::track_state(eState state) {
    if (state == Loading) ++s_load_generation;
    if (state == Running && !s_running_since) // also re-sent after level transitions
        s_running_since = GetTickCount64();
}

// MCC ends every session with a restart after the exit states. The clock is cleared there,
// not on the exit states: dropping players while the engine is still shutting down can hang
// it, and the next session queries its players before its own loading state.
void CGameManager::end_session() {
    s_running_since = 0;
}

static int deferred_player_count(int count) {
    if (count <= kDeferredJoinStartPlayers) return count;

    auto p_global = GameGlobal();
    if (p_global == nullptr) return count;

    auto game = p_global->current_game;
    if (game != CGameGlobal::Halo4)
        return count;

    ULONGLONG since = s_running_since;
    if (since && GetTickCount64() - since >= kDeferredJoinDelayMs)
        return count;

    return kDeferredJoinStartPlayers;
}

int CGameManager::active_player_count() {
    static std::atomic<int> s_last_logged = -1;
    int active = deferred_player_count(AlphaRing::Global::MCC::Splitscreen()->player_count);

    if (active != s_last_logged) {
        s_last_logged = active;
        auto p_global = GameGlobal();
        LOG_INFO("Splitscreen: exposing {} local players (game {}, in game {})",
                 active, p_global ? (int)p_global->current_game : -1, MCC::IsInGame());
    }

    return active;
}

bool CGameManager::get_xbox_user_id(CGameManager *self, __int64 *pId, wchar_t *pName, int size, int index) {
    auto p_setting = AlphaRing::Global::MCC::Splitscreen();
    auto p_profile = get_profile(index);

    if (!p_setting->b_override || !index)
        return ppOriginal.get_xbox_user_id(self, pId, pName, size, index);

    if (index >= active_player_count())
        return false;

    if (pId)
        *pId = p_profile->id;

    if (pName)
        String::wstrcpy(pName, p_profile->name, size >> 1);

    return true;
}

bool CGameManager::get_key_state(CGameManager *self, DWORD index, input_data_t *p_input) {
    bool result;
    LARGE_INTEGER qpc;
    float delta_time = 0;
    CInputDevice* p_device;

    auto p_profile = AlphaRing::Global::MCC::Splitscreen();
    auto device_manager = DeviceManager();
    auto p_global = AlphaRing::Global::Global();

    memset(p_input, 0, sizeof(input_data_t));

    if (p_global->show_imgui) {
        if (p_global->pause_game_on_menu_shown)
            return false;
        else if (p_global->disable_input_on_menu_shown)
            return true;
    }

    if (!p_profile->b_override) {
        // MCC reads the pads itself; its player 0 is the first controller.
        XINPUT_STATE state;
        if (index == 0 && g_menuConfig.spawnMenuMask && AlphaRing::Input::GetXInputGetState(0, &state) &&
            MCC::Spawn::HandlePlayerInput(0, state.Gamepad))
            return true; // the player's spawn menu has the pad
        return ppOriginal.get_key_state(self, index, p_input);
    }

    if (index >= p_profile->player_count)
        return false;

    if (p_profile->b_player0_use_km && !index) {
        p_device = device_manager->p_input_device[4];
        device_manager->table->update_state(device_manager, 0, 0, false);
        QueryPerformanceCounter(&qpc);
        auto v1 = qpc.QuadPart - device_manager->qpc.QuadPart;
        auto v2 = device_manager->qpc.QuadPart - qpc.QuadPart;
        delta_time = fminf(fmaxf(MCC::DeltaTime(v1 >= v2 ? v2 : v1) * 1000.0,0.1), 1000.0);
        device_manager->qpc = qpc;
    } else {
        if ((p_device = get_controller(index)) == nullptr)
            return true;

        AlphaRing::Input::GetXInputGetState(p_device->input_user, &p_device->state);

        // While the player's spawn menu is open it has the pad; the game sees it released.
        if (MCC::Spawn::HandlePlayerInput(index, p_device->state.Gamepad))
            memset(&p_device->state.Gamepad, 0, sizeof(p_device->state.Gamepad));
    }

    result = p_device
            ->p_method_table
            ->set_state(p_device, delta_time, p_input);

    p_device
            ->p_method_table
            ->check(p_device);

    return result;
}

CUserProfile* CGameManager::get_player_profile(CGameManager *self, __int64 xid)  {
    auto index = get_index(xid);
    auto p_setting = AlphaRing::Global::MCC::Splitscreen();

    if (!p_setting->b_override)
        return ppOriginal.get_player_profile(self, xid);

    if (!p_setting->b_override_profile && ((!index) || (index && p_setting->b_use_player0_profile)))
        return ppOriginal.get_player_profile(self, get_xuid(0));

    if (p_setting->b_use_player0_profile)
        return &get_profile(0)->profile;

    return &get_profile(get_index(xid))->profile;
}

CGamepadMapping* CGameManager::retrive_gamepad_mapping(CGameManager *self, __int64 xid) {
    auto index = get_index(xid);
    auto p_setting = AlphaRing::Global::MCC::Splitscreen();

    if (!p_setting->b_override)
        return ppOriginal.retrive_gamepad_mapping(self, xid);

    if (!p_setting->b_override_profile && ((!index) || (index && p_setting->b_use_player0_profile)))
        return ppOriginal.retrive_gamepad_mapping(self, get_xuid(0));

    if (p_setting->b_use_player0_profile)
        return &get_profile(0)->mapping;

    return &get_profile(get_index(xid))->mapping;
}
