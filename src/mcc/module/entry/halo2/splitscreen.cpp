// Left/Right split screen for Halo 2 (mcc/splitscreen/LeftRight). Halo 2 lays its views out on a grid it
// grows by rows or by columns, by a mode MCC's build always passes as 1 (rows: Top/Bottom); any other mode
// grows columns first, and with three players gives player 1 the full-height left half.
#include "halo2.h"

#include "mcc/CGameGlobal.h"
#include "mcc/splitscreen/LeftRight.h"

namespace Halo2::Entry::Splitscreen {
    namespace LeftRight = MCC::Splitscreen::LeftRight;

    int Mode(int views, int mode) { return LeftRight::OnScreen(CGameGlobal::Halo2, views) ? 2 : mode; }

    Halo2Entry(entry_grid, OFFSET_HALO2_PF_SPLIT_GRID, void, split_grid, int views, int mode, short* grid) {
        ((split_grid_t)entry_grid.m_pOriginal)(views, Mode(views, mode), grid);
    }

    Halo2Entry(entry_cell, OFFSET_HALO2_PF_SPLIT_CELL, void, split_cell, int view, int views, int mode,
               const short* grid, short* cell, short* span) {
        ((split_cell_t)entry_cell.m_pOriginal)(view, views, Mode(views, mode), grid, cell, span);
    }
}
