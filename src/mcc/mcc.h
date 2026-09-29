#pragma once

namespace MCC {
    bool Initialize();
    // Initialize has finished: before that, MCC's globals aren't found yet (a frame can be presented first).
    bool Ready();
    float DeltaTime(__int64 a1);
    bool IsInGame();
}
