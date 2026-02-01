#include "CGameManager.h"

#include "common.h"

#include <cstdio>
#include <guiddef.h>
#include <combaseapi.h>

static struct ProfileContainer_t {CGameManager::Profile_t profiles[4]; ProfileContainer_t();} container;

CGameManager::Profile_t* CGameManager::get_profile(int index) {return container.profiles + index;}

// Initialize default Xbox controller mapping for standard Halo controls
static void InitializeDefaultMapping(CGamepadMapping& mapping) {
    // Set all to a safe default first (RightTrigger for unmapped actions)
    for (int i = 0; i < 66; i++) {
        mapping.actions[i] = CGamepadMapping::RightTrigger;
    }

    // Standard Xbox Halo controls
    mapping.actions[0]  = CGamepadMapping::A;             // Jump
    mapping.actions[1]  = CGamepadMapping::LeftShoulder;  // Switch Grenades
    mapping.actions[2]  = CGamepadMapping::X;             // Action/Interact
    mapping.actions[3]  = CGamepadMapping::RightShoulder; // Reload Right Weapon
    mapping.actions[4]  = CGamepadMapping::Y;             // Change Weapon
    mapping.actions[5]  = CGamepadMapping::B;             // Melee
    mapping.actions[6]  = CGamepadMapping::DpadUp;        // Toggle Flashlight
    mapping.actions[7]  = CGamepadMapping::LeftTrigger;   // Throw Grenade
    mapping.actions[8]  = CGamepadMapping::RightTrigger;  // Use Right Weapon (Shoot)
    mapping.actions[9]  = CGamepadMapping::LeftThumb;     // Crouch
    mapping.actions[10] = CGamepadMapping::RightThumb;    // Player Zoom
    // 11, 12 are nullptr/unused
    mapping.actions[13] = CGamepadMapping::RightShoulder; // Swap/Reload Left Weapon
    mapping.actions[14] = CGamepadMapping::LeftThumb;     // Sprint (click left stick)
    mapping.actions[15] = CGamepadMapping::B;             // Banshee Bomb
    // 16-19 are movement (handled by sticks, not buttons)
    mapping.actions[20] = CGamepadMapping::Back;          // Multiplayer Scoreboard
    mapping.actions[21] = CGamepadMapping::DpadDown;      // Vehicle Function 2
    mapping.actions[22] = CGamepadMapping::DpadLeft;      // Vehicle Function 3
    mapping.actions[23] = CGamepadMapping::RightShoulder; // Use Equipment
    mapping.actions[24] = CGamepadMapping::DpadRight;     // Vehicle Function 1
    mapping.actions[27] = CGamepadMapping::X;             // Drop/Pickup
    mapping.actions[28] = CGamepadMapping::A;             // Thrust
    mapping.actions[49] = CGamepadMapping::LeftTrigger;   // Use Left Weapon
    mapping.actions[57] = CGamepadMapping::X;             // Special Action
    mapping.actions[58] = CGamepadMapping::Y;             // Open Loadouts Menu
    mapping.actions[64] = CGamepadMapping::DpadLeft;      // Select Previous Grenades
    mapping.actions[65] = CGamepadMapping::DpadRight;     // Select Next Grenades
}

ProfileContainer_t::ProfileContainer_t() {
    __int64 guid[2];
    const int controller_map[4] {0, 1, 2, 3};
    memset(this, 0, sizeof(ProfileContainer_t));

    CoCreateGuid((GUID*)guid);
    auto id = guid[0] ^ guid[1];

    for (int i = 0; i < 4; i++) {
        profiles[i].controller_index = controller_map[i];
        profiles[i].id = id + i;
        swprintf(profiles[i].name, L"Player %d", i + 1);

        // Initialize with standard Xbox Halo controls
        InitializeDefaultMapping(profiles[i].mapping);
    }
}

CGameManager* pGameManager;
CGameManager::FunctionTable CGameManager::ppOriginal;

bool CGameManager::Initialize(CGameManager* mng) {
    pGameManager = mng;
    return AlphaRing::Hook::Detour({
        {pGameManager->table->get_player_profile, get_player_profile, (void**)&ppOriginal.get_player_profile},
        {pGameManager->table->get_key_state, get_key_state, (void**)&ppOriginal.get_key_state},
        {pGameManager->table->get_xbox_user_id, get_xbox_user_id, (void**)&ppOriginal.get_xbox_user_id},
        {pGameManager->table->set_vibration, set_vibration, (void**)&ppOriginal.set_vibration},
        {pGameManager->table->retrive_gamepad_mapping, retrive_gamepad_mapping, (void**)&ppOriginal.retrive_gamepad_mapping},
        {pGameManager->table->set_state, set_state, (void**)&ppOriginal.set_state},
        {pGameManager->table->game_restart, game_restart, (void**)&ppOriginal.game_restart},
        {pGameManager->table->game_setup, game_setup, (void**)&ppOriginal.game_setup},
    });
}

__int64 CGameManager::get_xuid(int index) {
    __int64 result;

    if (index)
        return container.profiles[index].id;
    else
        return pGameManager->ppOriginal.get_xbox_user_id(pGameManager, &result, nullptr, 0, index) ? result : 0;
}

CInputDevice *CGameManager::get_controller(int index) {
    auto mng = DeviceManager();
    auto setting = AlphaRing::Global::MCC::Splitscreen();
    auto controller_index = get_profile(index)->controller_index;

    if ((!index && setting->b_player0_use_km) || controller_index >= 4 || controller_index < 0)
        return nullptr;
    else
        return mng->p_input_device[controller_index];
}

int CGameManager::get_index(__int64 xuid) {
    for (int i = 1; i < 4; ++i)
        if (container.profiles[i].id == xuid)
            return i;
    return 0;
}

void CGameManager::set_state(CGameManager *self, eState state) {
    auto state_name = "Unknown";
    if (state == Exiting)
        state_name = "Exiting";
    LOG_INFO("Set Game State[{}]: {}", state, state_name);
    return ppOriginal.set_state(self, state);
}

void *CGameManager::game_restart(CGameManager *self, int type, const char *reason) {
    auto final_reason = reason ? reason : "NoReason";
    LOG_INFO("Game Restart[{}]: {}", type, final_reason);
    return ppOriginal.game_restart(self, type, reason);
}

char __fastcall CGameManager::game_setup(CGameManager* self, void* a2) {
//    LOG_INFO("game setup"); player init/add
    return ppOriginal.game_setup(self, a2);
}
