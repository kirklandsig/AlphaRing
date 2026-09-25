#pragma once

#define OFFSET_HALO2_PF_LOAD 0x4F980
#define OFFSET_HALO2_PF_GAME 0x67A220
#define OFFSET_HALO2_PF_COPY_GAME_OPTIONS 0x39CE0 // copy_game_options(game_options)
#define OFFSET_HALO2_PF_HS_COMPILE 0x6E17C0 // compile a console script: (const char* text, bool)

#define OFFSET_HALO2_PF_ADD_LOCAL_PLAYER 0x69D2C0//0x69D290

#define OFFSET_HALO2_PV_PLAYERS 0xE80A28//0xE7FA28
#define OFFSET_HALO2_PV_RESPAWN 0xE80A20//0xE7FA20

#define OFFSET_HALO2_PF_PLAYER_VALID 0x6A6C80//0x6A6C30
#define OFFSET_HALO2_PF_PLAYER_COUNT1 0x8940CA//0x893FDA
#define OFFSET_HALO2_PF_PLAYER_COUNT2 0x894127//0x894037

// spawning
#define OFFSET_HALO2_PV_TAG_INSTANCES 0x15E4B30 // {u32 group, u32 datum, u32 address, u32 size}[tag count]
#define OFFSET_HALO2_PV_TAG_COUNT 0x15E4B60
#define OFFSET_HALO2_PV_TAG_NAME_OFFSETS 0x15E4B68 // int32[tag count] into the name buffer
#define OFFSET_HALO2_PV_TAG_NAMES 0x15E4B78
#define OFFSET_HALO2_PV_TAG_BASE 0xE80AB0 // tag block address -> map tag data + address
#define OFFSET_HALO2_PV_SHARED_TAG_BASE 0xE80AC0 // addresses with the top bit set are in the shared map
#define OFFSET_HALO2_PV_SCENARIO 0xE6F768
#define OFFSET_HALO2_PV_PLAYERS 0xE80A28 // data array (data at +*(+0x48)): 0x224 each, unit +0x2C
#define OFFSET_HALO2_PV_OBJECTS 0x18B7398 // object headers: data array of 0xC, object data offset +8
#define OFFSET_HALO2_PV_OBJECT_MEMORY 0x18B7360 // object data = ((pool + 0x57) & ~0xF) + offset
#define OFFSET_HALO2_PF_OBJECT_GET_ORIGIN 0x8D6780 // (int object, real_point3d*)
#define OFFSET_HALO2_PF_OBJECT_PLACEMENT_DATA_NEW 0x8D8880 // (data* [0xC4], int tag, int owner, void*)
#define OFFSET_HALO2_PF_OBJECT_NEW 0x8D79D0 // int (data*)
#define OFFSET_HALO2_PF_AI_PLACE 0x618890 // (int ai_index); starting location = (3 << 30) | (squad << 16) | location
#define OFFSET_HALO2_PF_AI_LIVING_COUNT 0x619B10 // int (int ai_index); a squad's ai index is its index

// per-player HUD (mcc/hud)
#define OFFSET_HALO2_PF_HUD_ASPECT_LOCK 0x954DF2 // je (74 3C) that skips the centred 16:9 HUD frame when the profile's aspect lock is off
#define OFFSET_HALO2_PV_HUD_ASPECT_LOCK 0x197EE40 // bool: the profiles' LockMaxAspectRatio ("HUD anchor: Centered"), the last loaded wins
#define OFFSET_HALO2_PV_HUD_DRAWING_PLAYER 0x165C398 // int local player whose HUD is being drawn
#define OFFSET_HALO2_PV_HUD_VIEW_BOUNDS 0x165C298 // int16 top, left, bottom, right of that player's HUD (window bounds)
#define OFFSET_HALO2_PV_HUD_UNIT_SCALE 0xE14F28 // float: pixels per HUD offset unit (times the element's scale)
