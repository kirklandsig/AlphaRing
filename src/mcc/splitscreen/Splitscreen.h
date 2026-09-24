#pragma once

namespace MCC::Splitscreen {
    bool Initialize();
    void ImGuiContext();
}

namespace MCC::Splitscreen {
    // Halo CE and Halo 2's Anniversary renderer only ever drew two views, so sessions with
    // three or four players start in Classic: bit 0 of the first game-options byte selects
    // Anniversary visuals in both engines, and it is cleared only while the engine copies
    // the options, leaving MCC's own setting untouched.
    class ClassicGraphicsScope {
    public:
        explicit ClassicGraphicsScope(unsigned char* game_options);
        ~ClassicGraphicsScope();

    private:
        unsigned char* m_options = nullptr;
        unsigned char m_saved = 0;
    };
}
