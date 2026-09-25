#include "CGameManager.h"

#include "mcc/hud/Hud.h"

#include <cstddef>

// Halo 3, ODST and Reach call these host methods from their HUD drawing (MCC's own versions back
// a feature-flagged HUD editor and the colour-blind filter). MCC's answer is kept and the drawing
// player's HUD settings are added on top - see mcc/hud/Hud.h.
static_assert(offsetof(CGameManager::FunctionTable, get_hud_element_anchor) == 0x2F8);
static_assert(offsetof(CGameManager::FunctionTable, get_hud_element_transform) == 0x300);
static_assert(offsetof(CGameManager::FunctionTable, transform_hud_color) == 0x308);
static_assert(offsetof(CGameManager::FunctionTable, retrive_gamepad_mapping) == 0x3A0);

bool CGameManager::get_hud_element_transform(CGameManager* self, int element, float* dx, float* dy, float* scale) {
    bool result = ppOriginal.get_hud_element_transform(self, element, dx, dy, scale);
    if (!result) { *dx = 0.0f; *dy = 0.0f; *scale = 1.0f; }
    return MCC::Hud::Transform(element, dx, dy, scale) || result;
}

// Only asked after get_hud_element_transform answered true.
bool CGameManager::get_hud_element_anchor(CGameManager* self, int element, int* anchor) {
    if (ppOriginal.get_hud_element_anchor(self, element, anchor)) return true;
    return MCC::Hud::Anchor(element, anchor);
}

unsigned CGameManager::transform_hud_color(CGameManager* self, int user, unsigned argb) {
    return MCC::Hud::Color(user, ppOriginal.transform_hud_color(self, user, argb));
}
