#include "CXboxContext.h"
#include "CXboxColorMapping.h"

#include "assets/open_wav.h"
#include "assets/close_wav.h"
#include "assets/nav_wav.h"
#include "assets/nav_page_wav.h"
#include "assets/select_wav.h"
#include "assets/denied_wav.h"

#include "mcc/CGameManager.h"
#include "global/Global.h"

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
        const MenuState& ms = m_stateMachine->getState().menuState;
        saveMenuStateBin(ms, k_savePath);

        CGameManager::apply_profiles();

        // apply controller indices and team assignments from UI state to each player profile
        auto* engine = GameEngine();
        for (int i = 0; i < 4; ++i) {
            auto profile = CGameManager::get_profile(i);
            if (profile)
                profile->controller_index = ms.controllerIndex[i];

            auto xuid = CGameManager::get_xuid(i);
            if (xuid && engine)
                engine->change_team(xuid, ms.teamIndex[i]);
        }

        // propagate splitscreen settings from UI state
        auto p_setting = AlphaRing::Global::MCC::Splitscreen();
        if (p_setting) {
            if (ms.playerCount > 1)
                p_setting->b_override = true;
            p_setting->player_count = ms.playerCount;
            p_setting->b_player0_use_km = ms.useKM;
            p_setting->b_use_player0_profile = (ms.playerCount <= 1);
        }

        // apply player colors for the current game
        auto* gg = GameGlobal();
        int game = gg ? static_cast<int>(gg->current_game) : static_cast<int>(CGameGlobal::Halo3);
        for (int i = 0; i < 4; ++i) {
            auto profile = CGameManager::get_profile(i);
            if (profile) {
                int primary   = CXboxColorMapping::GetColorIndex(ms.playerColors[i].colors[0], game);
                int secondary = CXboxColorMapping::GetColorIndex(ms.playerColors[i].colors[1], game);
                int tertiary  = CXboxColorMapping::GetColorIndex(ms.playerColors[i].colors[2], game);
                profile->profile.PlayerModelPrimaryColorIndex   = primary;
                profile->profile.PlayerModelSecondaryColorIndex = secondary;
                profile->profile.PlayerModelTertiaryColorIndex  = tertiary;
                profile->profile.PlayerModelPrimaryColor        = CXboxColorMapping::GetColorItemIndex(ms.playerColors[i].colors[0], game);
                profile->profile.PlayerModelSecondaryColor      = CXboxColorMapping::GetColorItemIndex(ms.playerColors[i].colors[1], game);
                profile->profile.PlayerModelTertiaryColor       = CXboxColorMapping::GetColorItemIndex(ms.playerColors[i].colors[2], game);
            }
        }
        if (engine)
            engine->load_setting();

        m_stateMachine.reset();
    }
}
