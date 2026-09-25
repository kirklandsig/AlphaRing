#pragma once

#define OFFSET_HALO1_PF_RENDER 0x1F05B0
#define OFFSET_HALO1_PF_GAME_START 0x939D0 // engine game_start(self, manager, game_options)
#define OFFSET_HALO1_PF_HS_COMPILE 0xB21470 // compile + run a console script: (const char* text)

#define OFFSET_HALO1_PF_4PLAYERS 0x67492
#define OFFSET_HALO1_PF_PAUSE 0x427978
#define OFFSET_HALO1_PF_IDK 0x85428

#define OFFSET_HALO1_PV_PLAYER_COUNT 0x1B7B910

// spawning (found through the retail cheat_spawn_warthog / cheat_all_chars / ai_attach_free script functions)
#define OFFSET_HALO1_PV_PLAYERS 0x1C40480 // players data array*, element 0xC20, unit index at +0x64
#define OFFSET_HALO1_PF_TAG_ITERATOR_NEXT 0xA9B43C // int (tag_iterator*)
#define OFFSET_HALO1_PF_TAG_GET_NAME 0xA9B25C // const char* (int tag)
#define OFFSET_HALO1_PF_TAG_GET 0xA9B648 // void* (int tag) - tag definition data
#define OFFSET_HALO1_PF_OBJECT_PLACEMENT_DATA_NEW 0xB35EBC // (data*, int tag, int owner_object)
#define OFFSET_HALO1_PF_OBJECT_NEW 0xB35F80 // int (data*)
#define OFFSET_HALO1_PF_OBJECT_GET_ORIGIN 0xB37DE0 // (int object, vector3* out)
#define OFFSET_HALO1_PF_OBJECT_GET_ORIENTATION 0xB37F98 // (int object, vector3* forward, vector3* up)
#define OFFSET_HALO1_PF_ACTOR_CUSTOMIZE_UNIT 0xC01DD0 // (int actor_variant, int unit) - weapon, grenades, colors
#define OFFSET_HALO1_PF_AI_ATTACH_FREE 0xBFF120 // (int unit, int actor_variant) - creates an encounterless actor

// per-player HUD (mcc/hud)
#define OFFSET_HALO1_PF_HUD_CALCULATE_POINT 0xB56A58 // (player, placement, header, -, bool, float, short2* out, int mcc element)
#define OFFSET_HALO1_PV_HUD_VIEWPORT_BOUNDS 0x29AF2F0 // int16 top, left, bottom, right of the view being drawn
#define OFFSET_HALO1_PV_HUD_DRAWING_PLAYER 0x29AF2B8 // int16 local player whose HUD is being drawn
