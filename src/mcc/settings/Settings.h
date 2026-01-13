#pragma once

#include <string>
#include "../CGameManager.h"

namespace MCC::Settings {

    struct SplitscreenConfig {
        bool b_override = false;
        int  player_count = 1;
        bool b_use_player0_profile = true;
        bool b_player0_use_km = false;
        bool b_override_profile = false;
    };

    namespace Splitscreen {
        bool Load();
        bool Save();

        void ApplyToRuntime();
        void CaptureFromRuntime();

        const SplitscreenConfig& Get();
    }

    namespace Profile {
        bool Load();
        bool Save();

        void ApplyToRuntime();
        void CaptureFromRuntime();
    }
}
