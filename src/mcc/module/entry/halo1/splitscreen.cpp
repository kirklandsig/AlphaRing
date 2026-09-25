// Left/Right split screen for Halo CE (mcc/splitscreen/LeftRight): CE lays its views out on a grid that
// always grows by rows first; two players get it turned into two columns.
#include "halo1.h"

#include "mcc/CGameGlobal.h"

#include "mcc/splitscreen/LeftRight.h"

namespace Halo1::Entry::Splitscreen {
    Halo1Entry(entry_grid, OFFSET_HALO1_PF_SPLIT_GRID, void, split_grid, int views, int* columns, int* rows) {
        ((split_grid_t)entry_grid.m_pOriginal)(views, columns, rows);
        if (MCC::Splitscreen::LeftRight::OnScreen(CGameGlobal::Halo1, views)) {
            *columns = 2;
            *rows = 1;
        }
    }
}
