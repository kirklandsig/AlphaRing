#pragma once

#include "render/imgui/ICContext.h"
#include <SDL_mixer.h>
#include "CStateMachine.h"

#include <array>
#include <optional>

class CXboxContext : public ICContext {
public:
    CXboxContext(ImFont* font);
    ~CXboxContext();

    void open();
    void close();
    bool isOpen() const;

    void handleInput(InputCommand cmd);
    void render() override;

private:
    Menu m_menu;
    std::array<Mix_Chunk*, 6> m_sounds;
    std::optional<StateMachine> m_stateMachine;
    ImFont* m_font;
    Uint32 m_lastTick = 0;

    static constexpr const char* k_savePath = "./MCC/Binaries/Win64/alpha_ring_menu.bin";
};

extern CXboxContext* g_pXboxContext;
