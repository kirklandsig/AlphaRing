#include "halo2.h"

#include "mcc/CGameGlobal.h"
#include "mcc/spawn/Command.h"

namespace Halo2::Entry::Console {
    // Game-thread entry for AlphaRing commands, see mcc/spawn/Command.h.
    Halo2Entry(entry, OFFSET_HALO2_PF_HS_COMPILE, bool, detour, const char* text, bool flag) {
        if (MCC::Command::Dispatch(CGameGlobal::Halo2, text))
            return false;
        return ((detour_t)entry.m_pOriginal)(text, flag);
    }
}
