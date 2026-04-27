#pragma once

#include <string>
#include <vector>
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
        void Initialize(CGameManager* mng);
    }

    namespace CustomMapping {
        // Get list of saved custom mapping profile names
        std::vector<std::string> GetProfileNames();

        // Save current mapping as a named profile
        bool SaveProfile(const std::string& name, const CGamepadMapping& mapping);

        // Load a named profile into the provided mapping
        bool LoadProfile(const std::string& name, CGamepadMapping& mapping);

        // Delete a named profile
        bool DeleteProfile(const std::string& name);
    }
}
