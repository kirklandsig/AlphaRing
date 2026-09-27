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

// Left/Right split screen (mcc/splitscreen/LeftRight)
#define OFFSET_HALO2_PF_SPLIT_GRID 0x7E09D0 // void (int views, int mode, short grid[2] columns, rows) - mode 1 stacks, others side by side
#define OFFSET_HALO2_PF_SPLIT_CELL 0x7E0BD0 // void (int view, int views, int mode, short grid[2], short cell[2], short span[2])
#define OFFSET_HALO2_PF_SPLIT_DIVIDERS 0x831D90 // void () - paints the black bands between the views, by the frame's mode
#define OFFSET_HALO2_PV_SPLIT_MODE 0x165C168 // int - the frame's grid mode, which only the dividers read
#define OFFSET_HALO2_PV_SPLIT_VIEWS 0x165C16C // int - the frame's views

// Anniversary graphics (Saber3D) with 3-4 players (module/entry/halo2/anniversary.cpp)
#define OFFSET_HALO2_PV_ANNIVERSARY 0xE21280 // int, nonzero while the Anniversary renderer is up
#define OFFSET_HALO2_PV_CLASSIC_SHOWN 0x1E8CE73 // bool, the player switched to Classic graphics
#define OFFSET_HALO2_PF_CINEMATIC 0x6F4A20 // bool () - one view (cinematics)
#define OFFSET_HALO2_PF_LOCAL_PLAYER_COUNT 0x6A4380 // int () - a 12-byte leaf
// returns from its "== 2" (split screen) tests: the views' hand-over, the object sync, a screen effect, and three
// first-person effect callbacks
#define OFFSET_HALO2_TWO_LOCAL_PLAYERS_TESTS 0x5153E, 0x6A00C, 0x6F40C, 0x64004, 0x64F4E, 0x65559
#define OFFSET_HALO2_PF_SABER_HAND_OVER_VIEW 0x5F510 // void (int view, bool split) - observer camera(view) -> cams[view]
#define OFFSET_HALO2_PV_SABER_CAMERAS 0x1E91210 // container*: camera*[] at +0x100, int count +0x108
#define OFFSET_HALO2_PF_SABER_CAMERA_SET_FOV 0xBC560 // void (camera*, float vertical degrees) - also the projection
#define OFFSET_HALO2_PV_SABER_DEVICE 0x1E8D418 // device*, settings* at +0x118: out w +0x10, h +0x14, pixel aspect +0x1C
#define OFFSET_HALO2_PV_SABER_CONTEXT 0x1E8D420 // context*, ID3D11DeviceContext* at +0xD58
#define OFFSET_HALO2_PV_SABER_SCENE 0x1A250F8 // scene*, the renderer's camera list at +0x140
#define OFFSET_HALO2_PF_SABER_BUILD_CAMERA_LIST 0x2DDCB0 // (renderer*, list*, bool split, -)
#define OFFSET_HALO2_PF_SABER_LIST_CAMERA 0x1C7740 // int (list*, camera*, flags, split index, 6 more) - appends a copy
#define OFFSET_HALO2_PF_SABER_RENDER_FRAME 0x2DEC00 // void ()
#define OFFSET_HALO2_PF_SABER_COMPOSITE 0x1D2A00 // void (texture*, bool split, int index) - a view's image to its half
#define OFFSET_HALO2_SABER_COMPOSITE_DRAW 0x1D2C33 // in it, the quad's vertices are at rsp+0xC0 (x, y, z, -, u, v; 0x18 each)
#define OFFSET_HALO2_PF_SABER_AFTER_CAMERAS 0x2E3F70 // void () - after the views, before the UI and HUD
#define OFFSET_HALO2_PF_PLAYER_VISIBLE_OBJECTS 0x608A0 // void (int local) - the window's visible objects, as view 0's (local 0) or view 1's
#define OFFSET_HALO2_PF_OWN_UNIT_IN_FIRST_PERSON 0x5FE80 // bool (int object, int local) - the local player's unit, in first person
#define OFFSET_HALO2_PF_FIRST_PERSON_SYNC 0x6D0D0 // void () - players 1-2's first-person models, each hidden in the other's view
#define OFFSET_HALO2_PF_FIRST_PERSON_BUILD 0x81BFB0 // void (int local)
#define OFFSET_HALO2_PV_FIRST_PERSON_MODELS 0x1AB11C0 // entries*, int count at +8; 0x28 each: +8 object**, +0x1C owner, +0x20 active
#define OFFSET_HALO2_PV_MODEL_LEVELS 0x1DC36F0 // unsigned char* detail levels, int count at +8 - the current one is the last
#define OFFSET_HALO2_PF_SABER_OBJECT_SHOW 0x69C30 // void (object*)
#define OFFSET_HALO2_PF_SABER_OBJECT_HIDE 0x69CA0 // void (object*)
