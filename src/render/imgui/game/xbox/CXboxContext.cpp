#include "CXboxContext.h"

#include "assets/open_wav.h"
#include "assets/close_wav.h"
#include "assets/nav_wav.h"
#include "assets/nav_page_wav.h"
#include "assets/select_wav.h"
#include "assets/denied_wav.h"

#include <SDL.h>
#include <imgui.h>

CXboxContext* g_pXboxContext = nullptr;

CXboxContext::CXboxContext(ImFont* font)
    : m_sounds{
        Mix_LoadWAV_RW(SDL_RWFromConstMem(open_wav,     open_wav_len),     1),
        Mix_LoadWAV_RW(SDL_RWFromConstMem(close_wav,    close_wav_len),    1),
        Mix_LoadWAV_RW(SDL_RWFromConstMem(nav_wav,      nav_wav_len),      1),
        Mix_LoadWAV_RW(SDL_RWFromConstMem(nav_page_wav, nav_page_wav_len), 1),
        Mix_LoadWAV_RW(SDL_RWFromConstMem(select_wav,   select_wav_len),   1),
        Mix_LoadWAV_RW(SDL_RWFromConstMem(denied_wav,   denied_wav_len),   1),
    },
    m_font(font)
{
}

CXboxContext::~CXboxContext() {
    if (m_stateMachine.has_value())
        saveMenuStateBin(m_stateMachine->getState().menuState, k_savePath);
    for (auto* chunk : m_sounds)
        Mix_FreeChunk(chunk);
}

void CXboxContext::open() {
    m_stateMachine.emplace(m_menu, m_sounds);
    loadMenuStateBin(m_stateMachine->getState().menuState, k_savePath);
    Mix_PlayChannel(-1, m_sounds[0], 0);
    m_lastTick = SDL_GetTicks();
}

void CXboxContext::close() {
    if (m_stateMachine.has_value())
        m_stateMachine->handleInput(InputCommand::Back);
}

bool CXboxContext::isOpen() const {
    return m_stateMachine.has_value();
}

void CXboxContext::handleInput(InputCommand cmd) {
    if (m_stateMachine.has_value())
        m_stateMachine->handleInput(cmd);
}

void CXboxContext::render() {
    if (!m_stateMachine.has_value()) return;

    Uint32 now = SDL_GetTicks();
    float dt = (now - m_lastTick) / 1000.0f;
    m_lastTick = now;

    m_stateMachine->update(dt);

    auto* vp = ImGui::GetMainViewport();
    m_stateMachine->render((int)vp->Size.x, (int)vp->Size.y, m_font);

    if (!m_stateMachine->isRunning()) {
        saveMenuStateBin(m_stateMachine->getState().menuState, k_savePath);
        m_stateMachine.reset();
    }
}
