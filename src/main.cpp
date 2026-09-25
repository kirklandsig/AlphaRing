#include "common.h"

#include "input/Input.h"
#include "hook/Hook.h"
#include "render/Render.h"
#include "mcc/mcc.h"

static bool Initialize() {
    bool result;

    result = AlphaRing::Log::Init();

    assertm(result, "failed to initialize log");

    // An MCC build these offsets weren't made for (an update, the Store version) gets no hooks
    // at all: it runs unmodified instead of crashing. (assertm is compiled out of Release.)
    if (!AlphaRing::Hook::Initialize()) {
        LOG_ERROR("AlphaRing disabled: this MCC build isn't supported, so the game runs unmodified");
        return false;
    }

    //LOG_INFO("Initialized AlphaRing.");
    
    result = AlphaRing::Filesystem::Init();

    assertm(result, "failed to initialize filesystem");

	//LOG_INFO("Initialized filesystem.");

    result = AlphaRing::Input::Init();

    assertm(result, "failed to initialize input");

	//LOG_INFO("Initialized input.");

    result = AlphaRing::Render::Initialize();

    assertm(result, "failed to initialize render");

	//LOG_INFO("Initialized render.");

    result = MCC::Initialize();

    assertm(result, "failed to initialize mcc");

	//LOG_INFO("Initialized mcc.");

    //LOG_INFO("Game Version[{}]: {}", AlphaRing::Hook::IsWS() ? "Windows Store" : "Steam", GAME_VERSION);

    return true;
}

static bool Shutdown() {
    LOG_INFO("Shutting down");

    AlphaRing::Filesystem::Shutdown();
    AlphaRing::Input::Shutdown();
    AlphaRing::Hook::Shutdown();
    AlphaRing::Log::Shutdown();

    return true;
}

BOOL APIENTRY DllMain(HANDLE handle, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Initialize, nullptr, 0, nullptr);
    } else if (reason == DLL_PROCESS_DETACH) {
        if (reserved == nullptr)
            return Shutdown();
    }

    return true;
}