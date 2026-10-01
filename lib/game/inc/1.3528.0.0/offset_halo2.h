#pragma once

#define OFFSET_HALO2_PF_LOAD 0x4F980
#define OFFSET_HALO2_PF_GAME 0x67A220

#define OFFSET_HALO2_PF_ADD_LOCAL_PLAYER 0x69D2C0//0x69D290

#define OFFSET_HALO2_PV_PLAYERS 0xE80A28//0xE7FA28
#define OFFSET_HALO2_PV_RESPAWN 0xE80A20//0xE7FA20

#define OFFSET_HALO2_PF_PLAYER_VALID 0x6A6C80//0x6A6C30
#define OFFSET_HALO2_PF_PLAYER_COUNT1 0x8940CA//0x893FDA
#define OFFSET_HALO2_PF_PLAYER_COUNT2 0x894127//0x894037

// In the main loop's quit handling (0x679CD0): call 0x6A6320 (did this game come from a Halo 2 lobby?), then
// test al, al / je - true loads the main-menu map to go back to that lobby, false quits to MCC.
#define OFFSET_HALO2_PF_QUIT_TO_LOBBY_TEST 0x679EE0
