#pragma once

namespace MCC::Splitscreen {
    bool Initialize();
    void ImGuiContext();
    // The Players window (Join.cpp): controllers join and leave with A and B.
    void JoinContext();
}

namespace MCC::Splitscreen {
    // Halo CE and Halo 2's Anniversary renderer only ever drew two views, stacked, so missions with
    // three or four players, or side by side (LeftRight.h, which takes the layout choice here), start in
    // Classic: bit 0 of the first game-options byte selects Anniversary visuals in both engines, and it
    // is cleared only while the engine copies the options, leaving MCC's own setting untouched. With 3-4
    // players (not side by side) both can keep Anniversary instead, an experimental choice
    // (module/entry/halo1/anniversary.cpp, module/entry/halo2/anniversary.cpp).
    class ClassicGraphicsScope {
    public:
        // `game`: CGameGlobal::Halo1 or Halo2
        ClassicGraphicsScope(unsigned char* game_options, int game);
        ~ClassicGraphicsScope();

    private:
        unsigned char* m_options = nullptr;
        unsigned char m_saved = 0;
    };
}

namespace MCC::Splitscreen {
    // Halo CE and Halo 2 with 3-4 players in Anniversary graphics: everyone's saved choice, which takes effect
    // as a mission starts, and whether the current mission started with it.
    bool AnniversaryQuadGame(int game); // Halo CE and Halo 2
    bool AnniversaryQuadChosen();
    void ChooseAnniversaryQuad(bool on);
    bool AnniversaryQuadActive();
}

namespace MCC::Splitscreen {
    // Hot join (experimental): players join in the middle of a mission, and in Halo CE leave (Join.cpp). The saved
    // choice with AlphaRing's split screen on (Halo CE's and Halo 2's modules take it as a mission starts); whether
    // the running mission takes joins, and leaves; each frame, a controller no player uses joining with A.
    bool HotJoinOn();
    bool HotJoinActive();
    bool HotJoinLeaves();
    void HotJoinPoll();
    // Local players MCC is told of: all four slots in a hot-join Halo CE or Halo 2 mission, else the player count.
    int HotJoinSlots(int player_count);
}
