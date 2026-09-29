#include "mcc.h"

#include <offset_mcc.h>

#include "CGameManager.h"
#include "CGameGlobal.h"

#include "mcc/module/Module.h"
#include "mcc/network/Network.h"
#include "mcc/splitscreen/Splitscreen.h"
#include "mcc/settings/Settings.h"
#include "mcc/hud/Hud.h"

#include <atomic>

namespace MCC {
    static bool* bIsInGame;
    static float (__fastcall* deltaTime)(long long qpc);
    static std::atomic<bool> s_ready; // Initialize finished (it runs on its own thread, after the render hooks)

    bool Ready() { return s_ready; }

    float DeltaTime(__int64 a1) {
        return deltaTime(a1);
    }

    bool IsInGame() {
        return *bIsInGame;
    }

    bool Initialize() {
        bool result;
        CGameEngine** ppGameEngine;
        CGameManager* game_manager;
        CDeviceManager** device_manager;

        AlphaRing::Hook::Offset({
            {OFFSET_MCC_PV_GAME_ENGINE, OFFSET_MCC_WS_PV_GAME_ENGINE, (void**)&ppGameEngine},
            {OFFSET_MCC_PV_GAME_MANAGER, OFFSET_MCC_WS_PV_GAME_MANAGER, (void**)&game_manager},
            {OFFSET_MCC_PV_DEVICE_MANAGER, OFFSET_MCC_WS_PV_DEVICE_MANAGER, (void**)&device_manager},
            {OFFSET_MCC_PF_DELTA_TIME, OFFSET_MCC_WS_PF_DELTA_TIME, (void**)&deltaTime},
            {OFFSET_MCC_PV_IS_IN_GAME, OFFSET_MCC_WS_PV_IS_IN_GAME, (void**)&bIsInGame},
            {OFFSET_MCC_PV_GAME_GLOBAL, OFFSET_MCC_WS_PV_GAME_GLOBAL, (void**)&g_ppGameGlobal},
        });

        assertm(ppGameEngine != nullptr, "MCC: failed to get ppGameEngine");
        assertm(game_manager != nullptr, "MCC: failed to get pGameManager");
        assertm(device_manager != nullptr, "MCC: failed to get ppDeviceManager");

        result = CGameEngine::Initialize(ppGameEngine);

        assertm(result, "MCC: failed to initialize GameEngine");

        result = CGameManager::Initialize(game_manager);

        assertm(result, "MCC: failed to initialize GameManager");

        assertm(GameManager() != nullptr, "MCC:Splitscreen: GameManager is null"); // static instance

        result = CDeviceManager::Initialize(device_manager);

        assertm(result, "MCC: failed to initialize DeviceManager");

        if (!Module::Initialize())
        {
			MessageBox(nullptr, "MCC: failed to initialize Module", "Error", MB_OK);
            return false;
        }

        if (!Splitscreen::Initialize())
        {
			MessageBox(nullptr, "MCC: failed to initialize Splitscreen", "Error", MB_OK);
            return false;
        }

        MCC::Settings::Splitscreen::Load();
        bool profileLoad = MCC::Settings::Profile::Load();
        if(profileLoad) {
            MCC::Settings::Profile::ApplyToRuntime();
            // MCC::Settings::Profile::Initialize(game_manager);
        }
        MCC::Settings::Splitscreen::ApplyToRuntime();
        MCC::Hud::Load();

		////Ask user if they want to enable network
  //      if (MessageBox(nullptr, "Would you like to enable network?", "Network", MB_YESNO) == IDYES)
  //      {
  //          if (!Network::Initialize())
  //          {
  //              MessageBox(nullptr, "MCC: failed to initialize Network", "Error", MB_OK);
  //              return false;
  //          }
  //      }

        s_ready = true;
        return true;
    }
}
