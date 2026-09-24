#include "halo1.h"

#include "mcc/CGameGlobal.h"
#include "mcc/spawn/Command.h"

namespace Halo1::Entry::Console {
    // Game-thread entry for AlphaRing commands, see mcc/spawn/Command.h.
    Halo1Entry(entry, OFFSET_HALO1_PF_HS_COMPILE, void, detour, const char* text) {
        if (MCC::Command::Dispatch(CGameGlobal::Halo1, text))
            return;
        ((detour_t)entry.m_pOriginal)(text);
    }
}
