// Halo CE Anniversary graphics with 3-4 players (experimental).
//
// The Anniversary (Saber3D) renderer draws split screen as exactly two views stacked: two cameras, render targets
// with two half-height children (_SPLIT_1/_SPLIT_2) that a two-way switch picks between, a composite that copies
// each view's image into its half of the screen, and a good deal of per-view state with two slots. Quad mode keeps
// that two-view frame and alternates what is in it: players 1 and 2 on one frame, 3 and 4 on the next. Every
// split target gets quarter-size children swapped in for the frame, the cameras get quarter viewports, and each
// view is copied into its quadrant of an image of our own, which is copied to the screen before the HUD.
#include "halo1.h"

#include "mcc/module/entry/PreservingThunk.h"
#include "mcc/splitscreen/Splitscreen.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace Halo1::Entry::Anniversary {
    EntryFeature("Halo CE Anniversary graphics with 3-4 players",
                 OFFSET_HALO1_PV_PLAYER_COUNT, OFFSET_HALO1_PV_SABER_DEVICE, OFFSET_HALO1_PV_SABER_CAMERAS,
                 OFFSET_HALO1_PF_SABER_DELETE_CAMERA, OFFSET_HALO1_PV_SABER_SPLIT, OFFSET_HALO1_PF_SABER_ADD_CAMERA,
                 OFFSET_HALO1_PF_SABER_CAMERA_SET_FOV, OFFSET_HALO1_PV_SABER_RENDERER, OFFSET_HALO1_PV_SABER_SPLIT_STATE,
                 OFFSET_HALO1_PF_SABER_SPLIT_LAYOUT, OFFSET_HALO1_PF_FIRST_PERSON_PREPARE,
                 OFFSET_HALO1_PV_FIRST_PERSON_FOR_RENDERER, OFFSET_HALO1_PF_FIRST_PERSON_UPDATE,
                 OFFSET_HALO1_PV_FIRST_PERSON_WEAPONS, OFFSET_HALO1_PV_FIRST_PERSON_WEAPON_NODES,
                 OFFSET_HALO1_PV_FIRST_PERSON_ARMS, OFFSET_HALO1_PV_FIRST_PERSON_ARMS_NODES, OFFSET_HALO1_PV_LOCAL_PLAYERS,
                 OFFSET_HALO1_PV_RENDER_WINDOWS, OFFSET_HALO1_PV_RENDER_CAMERAS, OFFSET_HALO1_PF_SPLIT_WINDOW,
                 OFFSET_HALO1_PV_WINDOW_FIELDS_OF_VIEW, OFFSET_HALO1_PF_RENDER_WINDOW_CAMERA,
                 OFFSET_HALO1_PF_SABER_HAND_OVER_VIEW, OFFSET_HALO1_PF_DIRECTOR_CAMERA_MODE,
                 OFFSET_HALO1_PV_HIDDEN_OBJECT_CAMERA, OFFSET_HALO1_PV_RENDER_OBJECTS, OFFSET_HALO1_PV_DIRECTORS,
                 OFFSET_HALO1_PF_FIRST_PERSON_DIRECTOR, OFFSET_HALO1_PF_LOCAL_PLAYER_UNIT_FLAG, OFFSET_HALO1_PV_SABER_SCENE,
                 OFFSET_HALO1_PV_SABER_ZOOMED_FOV, OFFSET_HALO1_PV_SABER_CONTEXT, OFFSET_HALO1_PV_SABER_VIEW_IMAGES,
                 OFFSET_HALO1_PV_SABER_VIEW_IMAGE_INDEX, OFFSET_HALO1_PV_SABER_SCREEN_IMAGE, OFFSET_HALO1_PF_HUD_VIEW_DRAW,
                 OFFSET_HALO1_PV_HUD_VIEW_DRAW_STATE, OFFSET_HALO1_PF_HUD_VIEW_DRAW_2, OFFSET_HALO1_PF_HUD_VIEW_DRAW_3,
                 OFFSET_HALO1_PV_HUD_VIEW_DRAW_3_SKIP_1, OFFSET_HALO1_PV_HUD_VIEW_DRAW_3_SKIP_2,
                 OFFSET_HALO1_PV_ANNIVERSARY_SHOWN);

    // Everything the mode hooks and reaches is in this build (MCC::Splitscreen::ClassicGraphicsScope asks).
    bool Available() { return entry_feature->Available(); }

    int LocalPlayers(__int64 module) { return *(short*)(module + OFFSET_HALO1_PV_PLAYER_COUNT); }
    int Views(__int64 module) { return std::clamp(LocalPlayers(module), 0, 4); }
    // The mode is set up for a 3-4 player mission (its cameras made as split screen starts) and runs while Anniversary
    // graphics are on screen: Back switches them in the middle of a mission.
    bool QuadReady(__int64 module) { return MCC::Splitscreen::AnniversaryQuadActive() && Views(module) > 2; }
    bool AnniversaryShown(__int64 module) { return *(int*)(module + OFFSET_HALO1_PV_ANNIVERSARY_SHOWN) != 0; }
    bool Quad(__int64 module) { return QuadReady(module) && AnniversaryShown(module); }

    char* Settings(__int64 module) { return *(char**)(*(char**)(module + OFFSET_HALO1_PV_SABER_DEVICE) + 0x118); }
    int ScreenWidth(__int64 module) { return *(int*)(Settings(module) + 0x20); }
    int ScreenHeight(__int64 module) { return *(int*)(Settings(module) + 0x24); }

    char** Cameras(__int64 module) { return *(char***)(module + OFFSET_HALO1_PV_SABER_CAMERAS); }
    int& CameraCount(__int64 module) { return *(int*)(module + OFFSET_HALO1_PV_SABER_CAMERAS + 8); }
    int CameraCapacity(__int64 module) { return *(int*)(module + OFFSET_HALO1_PV_SABER_CAMERAS + 0xC); }

    // The renderer's cameras 0 and 1, and ours for views 2 and 3 (below).
    char* s_extra_cameras[2] = {};
    char* Camera(__int64 module, int view) {
        if (view >= 2) return view < 4 ? s_extra_cameras[view - 2] : nullptr;
        return view >= 0 && view < CameraCount(module) ? Cameras(module)[view] : nullptr;
    }

    // ---- render targets: quarter-size children beside the split ones. The texture manager finds textures by
    // name, so ours carry their size, and a resize makes new ones.

    std::string SizedName(const char* name, int width, int height) {
        return std::string(name) + "_" + std::to_string(width) + "x" + std::to_string(height);
    }

    struct QuadChildren {
        char* parent;
        char* children[2];
    };
    std::mutex s_children_mutex;
    std::vector<QuadChildren> s_children;

    extern ::Entry entry_create_child;
    // the fifth argument is unused; the sixth's low word is the texture's flags (+0x7C)
    using create_child_t = char* (*)(char* parent, const char* name, int width, int height, __int64, int flags);

    bool EndsWith(const char* text, const char* suffix) {
        size_t n = strlen(text), m = strlen(suffix);
        return n >= m && strcmp(text + n - m, suffix) == 0;
    }

    char* CreateChild(char* parent, const char* name, int width, int height, __int64 unused, int flags) {
        auto original = (create_child_t)entry_create_child.m_pOriginal;
        char* child = original(parent, name, width, height, unused, flags);
        int which = EndsWith(name, "_SPLIT_1") ? 0 : EndsWith(name, "_SPLIT_2") ? 1 : -1;
        if (which >= 0 && child != nullptr) {
            char* quad = original(parent, SizedName((std::string(name) + "_QUAD").c_str(), width / 2, height).c_str(),
                                  width / 2, height, unused, flags);
            std::lock_guard<std::mutex> lock(s_children_mutex);
            auto it = std::find_if(s_children.begin(), s_children.end(), [&](auto& e) { return e.parent == parent; });
            if (it == s_children.end()) it = s_children.insert(s_children.end(), {parent, {nullptr, nullptr}});
            it->children[which] = quad;
        }
        return child;
    }
    ::Entry entry_create_child(Halo1EntrySet(), OFFSET_HALO1_PF_SABER_TEXTURE_CREATE_CHILD, (void*)&CreateChild,
                               entry_feature);

    void SwapChildren() {
        std::lock_guard<std::mutex> lock(s_children_mutex);
        for (auto& e : s_children) {
            if (e.children[0] == nullptr || e.children[1] == nullptr) continue;
            std::swap(*(char**)(e.parent + 0xA8), e.children[0]);
            std::swap(*(char**)(e.parent + 0xB0), e.children[1]);
        }
    }

    // ---- cameras: the renderer makes one for the second view; the third and fourth are ours. They are made by
    // the renderer's AddCamera and then kept past its camera count, so that everything it bounds by that count
    // stays with two views - its screen effects, for one, start in any camera below the count and write two-slot
    // arrays. Quarter viewports for all four.

    void DropExtraCameras(__int64 module) {
        for (char*& camera : s_extra_cameras) {
            if (camera != nullptr) ((void (*)(char*))(module + OFFSET_HALO1_PF_SABER_DELETE_CAMERA))(camera);
            camera = nullptr;
        }
    }

    // As split screen starts, or when a third player joins in the middle of a mission (hot join).
    void MakeExtraCameras(__int64 module) {
        int& count = CameraCount(module);
        if (!QuadReady(module) || s_extra_cameras[0] != nullptr || count != 2) return;
        for (int i = 0; i < 2; ++i) ((int (*)())(module + OFFSET_HALO1_PF_SABER_ADD_CAMERA))();
        if (count == 4) {
            s_extra_cameras[0] = Cameras(module)[2];
            s_extra_cameras[1] = Cameras(module)[3];
            LOG_INFO("Halo CE Anniversary 3-4 players: cameras for views 3 and 4 made");
        }
        count = 2;
    }

    Halo1Entry(entry_set_split, OFFSET_HALO1_PF_SABER_SET_SPLIT, void, set_split, char* state) {
        __int64 module = entry_set_split.m_target - entry_set_split.m_offset;
        if (!*(bool*)(module + OFFSET_HALO1_PV_SABER_SPLIT) && state[0x41]) DropExtraCameras(module); // turning off
        ((set_split_t)entry_set_split.m_pOriginal)(state);
        if (state[0x41]) MakeExtraCameras(module);
    }

    bool s_quarters = false; // the cameras have quarter viewports

    bool HasQuarters(__int64 module) {
        float width = ScreenWidth(module) * 0.5f, height = ScreenHeight(module) * 0.5f;
        for (int view = 0; view < 4; ++view)
            if (char* camera = Camera(module, view))
                if (*(float*)(camera + 0x144) != width || *(float*)(camera + 0x148) != height) return false;
        return true;
    }

    void QuarterLayout(__int64 module) {
        int width = ScreenWidth(module), height = ScreenHeight(module);
        float aspect = (float)height / (float)width / *(float*)(Settings(module) + 0x1C);
        for (int view = 0; view < 4; ++view) {
            char* camera = Camera(module, view);
            if (camera == nullptr) continue;
            *(float*)(camera + 0x154) = aspect;
            ((void (*)(char*, float))(module + OFFSET_HALO1_PF_SABER_CAMERA_SET_FOV))(camera, *(float*)(camera + 0x14C));
            *(__int64*)(camera + 0x13C) = 0;
            *(float*)(camera + 0x144) = width * 0.5f;
            *(float*)(camera + 0x148) = height * 0.5f;
            *(float*)(camera + 0x158) = width * 0.5f / *(float*)(camera + 0x134);
            *(float*)(camera + 0x15C) = height * 0.5f / *(float*)(camera + 0x138);
        }
        *(unsigned char*)(*(char**)(module + OFFSET_HALO1_PV_SABER_RENDERER) + 0x92) = 1;
    }

    // Leaving quad mode (or a list drawn whole, as cutscenes are): the renderer's own layout again.
    void Layout(__int64 module, bool quad, bool split) {
        if (quad) {
            if (!HasQuarters(module)) QuarterLayout(module);
            s_quarters = true;
        } else if (s_quarters) {
            s_quarters = false;
            char* state = *(char**)(module + OFFSET_HALO1_PV_SABER_SPLIT_STATE);
            if (state != nullptr) ((void (*)(char*, bool))(module + OFFSET_HALO1_PF_SABER_SPLIT_LAYOUT))(state, split);
        }
    }

    // ---- the pairs. Each frame the game hands its state to the renderer (the sync) and builds a camera list,
    // which the next frame's sync commits for the renderer to draw. The lists alternate pairs, players 1 and 2
    // then 3 and 4, and what the sync hands over for the views goes with the list it builds.

    std::atomic<int> s_pair = 0;           // the pair of the list the sync builds
    std::atomic<bool> s_sync_quad = false; // the sync is in quad mode
    std::atomic<int> s_listed_pair = 0;    // the pair of the list the renderer draws
    std::atomic<int> s_waiting_pair = 0;   // the pair of the list built aside, waiting to be committed

    Halo1Entry(entry_sync, OFFSET_HALO1_PF_SABER_SYNC, void, sync) {
        __int64 module = entry_sync.m_target - entry_sync.m_offset;
        char* split = *(char**)(module + OFFSET_HALO1_PV_SABER_SPLIT_STATE);
        if (split != nullptr && split[0x41]) MakeExtraCameras(module); // a third player joined (hot join)
        bool quad = Quad(module);
        s_sync_quad = quad;
        s_pair = quad ? s_pair ^ 1 : 0;
        ((sync_t)entry_sync.m_pOriginal)();
    }

    // ---- each frame the game hands its views 0 and 1 to the renderer's cameras: 2 and 3 likewise

    thread_local bool t_hand_over = false; // handing over
    thread_local bool t_view_block = false; // and each view's HUD view and first-person weapon were worked out

    // The first-person weapon of the view the HUD is set up for, for the renderer (views 0-3, the renderer takes 0-1).
    void FirstPersonView(__int64 module, int view) {
        ((void (*)(int))(module + OFFSET_HALO1_PF_HUD_VIEW))(view);
        ((void (*)())(module + OFFSET_HALO1_PF_FIRST_PERSON_PREPARE))();
        *(int*)(module + OFFSET_HALO1_PV_FIRST_PERSON_FOR_RENDERER) = 1;
        ((void (*)())(module + OFFSET_HALO1_PF_FIRST_PERSON_UPDATE))();
        *(int*)(module + OFFSET_HALO1_PV_FIRST_PERSON_FOR_RENDERER) = 0;
    }

    // Players 3 and 4's frame: their first-person weapons go to the renderer as views 0 and 1's.
    void FirstPersonPair(__int64 module, int views) {
        const std::pair<__int64, __int64> kModels[] = {
            {OFFSET_HALO1_PV_FIRST_PERSON_WEAPONS, OFFSET_HALO1_PV_FIRST_PERSON_WEAPON_NODES},
            {OFFSET_HALO1_PV_FIRST_PERSON_ARMS, OFFSET_HALO1_PV_FIRST_PERSON_ARMS_NODES},
        };
        for (int slot = 0; slot < 2; ++slot) {
            int view = slot + 2;
            for (auto [objects, nodes] : kModels) {
                int* object = (int*)(module + objects);
                object[slot] = view < views ? object[view] : -1;
                if (view < views) memcpy((char*)(module + nodes) + slot * 0xD00, (char*)(module + nodes) + view * 0xD00, 0xD00);
            }
        }
    }

    Halo1Entry(entry_hand_over, OFFSET_HALO1_PF_SABER_HAND_OVER_VIEWS, void, hand_over) {
        t_hand_over = true;
        t_view_block = false;
        ((hand_over_t)entry_hand_over.m_pOriginal)();
        t_hand_over = false;
        __int64 module = entry_hand_over.m_target - entry_hand_over.m_offset;
        if (!s_sync_quad) return;
        int players = *(short*)(*(char**)(module + OFFSET_HALO1_PV_LOCAL_PLAYERS) + 0xB4);
        int views = (std::min)(players, s_extra_cameras[1] != nullptr ? 4 : s_extra_cameras[0] != nullptr ? 3 : 2);
        // the hand-over (0xB29268 -> 0x7B480) writes the renderer's camera `view` if it is below the count: our
        // cameras stand in the array's slots past it for the moment
        int& count = CameraCount(module);
        if (views > 2 && CameraCapacity(module) >= 4 && count == 2) {
            Cameras(module)[2] = s_extra_cameras[0];
            Cameras(module)[3] = s_extra_cameras[1];
            count = views;
            for (int view = 2; view < views; ++view) {
                char* window = (char*)(module + OFFSET_HALO1_PV_RENDER_WINDOWS + view * 0xAC);
                char* camera = (char*)(module + OFFSET_HALO1_PV_RENDER_CAMERAS + view * 0x2A8);
                *(short*)window = (short)view;
                window[2] = 0;
                ((void (*)(int, int, short*, short*))(module + OFFSET_HALO1_PF_SPLIT_WINDOW))(
                    view, players, (short*)(window + 0x84), (short*)(window + 0x8C));
                *(float*)(module + OFFSET_HALO1_PV_WINDOW_FIELDS_OF_VIEW + (view + 1) * 4) = *(float*)(camera + 0x38);
                ((void (*)(char*, char*))(module + OFFSET_HALO1_PF_RENDER_WINDOW_CAMERA))(window, camera);
                if (t_view_block) FirstPersonView(module, view);
                ((void (*)(int))(module + OFFSET_HALO1_PF_SABER_HAND_OVER_VIEW))(view);
            }
            count = 2;
        }
        if (s_pair == 1) FirstPersonPair(module, views);
    }

    // ---- who is seen in which view. The renderer's objects have a hidden flag per view (0 and 1), which the sync
    // sets from the game: a local player's own unit is hidden in its view while in first person, and each player
    // has models of its own seen only in its view. The game answers for players 1 and 2; in quad mode, for the pair.

    int* LocalUnits(__int64 module) { return (int*)(*(char**)(module + OFFSET_HALO1_PV_LOCAL_PLAYERS) + 0xC8); }

    int Viewer(int slot) { return slot + 2 * s_pair; } // the local player seen through the view

    int CameraMode(__int64 module, int local) { // 0: first person
        return ((short (*)(int))(module + OFFSET_HALO1_PF_DIRECTOR_CAMERA_MODE))(local);
    }

    Halo1Entry(entry_hidden_views, OFFSET_HALO1_PF_OBJECT_HIDDEN_VIEWS, int, hidden_views, int object) {
        if (!s_sync_quad) return ((hidden_views_t)entry_hidden_views.m_pOriginal)(object);
        __int64 module = entry_hidden_views.m_target - entry_hidden_views.m_offset;
        int* units = LocalUnits(module);
        int hidden = 0;
        for (int slot = 0; slot < 2; ++slot)
            if (object != -1 && units[Viewer(slot)] == object && CameraMode(module, Viewer(slot)) == 0) hidden |= 1 << slot;
        if (hidden != 0) return hidden;
        // and, as the game does, both for the object a special camera is set to hide
        bool hides = *(bool*)(module + OFFSET_HALO1_PV_HIDDEN_OBJECT_CAMERA) && *(short*)(module + OFFSET_HALO1_PV_HIDDEN_OBJECT_CAMERA + 2) == 2
                     && *(int*)(module + OFFSET_HALO1_PV_HIDDEN_OBJECT_CAMERA + 0x34) == object;
        return hides ? 3 : 0;
    }

    void Hide(char* object, int slot, bool hidden) {
        ((void (*)(char*, int))(*(void***)object)[(hidden ? 0x328 : 0x330) / 8])(object, slot);
    }

    // The players' own models: the first shown in their view in first person (unless seated), the second out of it.
    Halo1Entry(entry_sync_object, OFFSET_HALO1_PF_SABER_SYNC_OBJECT, void, sync_object, char* record) {
        ((sync_object_t)entry_sync_object.m_pOriginal)(record);
        if (!s_sync_quad) return;
        __int64 module = entry_sync_object.m_target - entry_sync_object.m_offset;
        char* objects = *(char**)(module + OFFSET_HALO1_PV_RENDER_OBJECTS);
        int object = *(int*)(objects + *(int*)(objects + 0x34) + *(unsigned short*)record * 0x7C + 0x2C);
        int* units = LocalUnits(module);
        int local = object == -1 ? 4 : int(std::find(units, units + 4, object) - units);
        if (local > 3) return;
        char* director = (char*)(module + OFFSET_HALO1_PV_DIRECTORS + local * 0xF8);
        bool first_person = *(__int64*)(director + 0x10) == module + OFFSET_HALO1_PF_FIRST_PERSON_DIRECTOR
                            && *(float*)(director + 0xC) <= 0.0f;
        bool seated = ((int (*)(int))(module + OFFSET_HALO1_PF_LOCAL_PLAYER_UNIT_FLAG))(local) != 0;
        char** first = *(char***)(record + 8);
        char** second = *(char***)(record + 0x10);
        for (int slot = 0; slot < 2; ++slot) {
            bool own = Viewer(slot) == local;
            if (first != nullptr && *first != nullptr) Hide(*first, slot, !(own && first_person && !seated));
            if (second != nullptr && *second != nullptr) Hide(*second, slot, !own || first_person || seated);
        }
    }

    // ---- the camera list: the pair's cameras in place of the first two

    thread_local int t_pair = -1; // the pair of the list being built

    char* RenderList(__int64 module) { return *(char**)(module + OFFSET_HALO1_PV_SABER_SCENE) + 0xB0; }

    Halo1Entry(entry_prepare, OFFSET_HALO1_PF_SABER_PREPARE_FRAME, void, prepare, char* scene) {
        if (*(int*)(scene + 0xBE58) && *(int*)(scene + 0xBE5C)) s_listed_pair = s_waiting_pair.load(); // committing
        ((prepare_t)entry_prepare.m_pOriginal)(scene);
    }

    Halo1Entry(entry_build_list, OFFSET_HALO1_PF_SABER_BUILD_CAMERA_LIST, __int64, build_list, void* a, char* list,
               __int64 split, void* d) {
        __int64 module = entry_build_list.m_target - entry_build_list.m_offset;
        bool quad = Quad(module) && (unsigned char)split;
        Layout(module, quad, (unsigned char)split);
        if (!quad) return ((build_list_t)entry_build_list.m_pOriginal)(a, list, split, d);
        int pair = s_pair;
        t_pair = pair;
        auto result = ((build_list_t)entry_build_list.m_pOriginal)(a, list, split, d);
        t_pair = -1;
        (list == RenderList(module) ? s_listed_pair : s_waiting_pair) = pair;
        return result;
    }

    // The list takes a copy of each camera and works its aspect out from the view's height over the screen's width,
    // which a quarter view is not: the copy gets a height it comes out right with, and its own back once listed.
    Halo1Entry(entry_list_camera, OFFSET_HALO1_PF_SABER_LIST_CAMERA, __int64, list_camera, char* list, char* camera,
               __int64 flags, __int64 index, __int64 a5, __int64 a6, __int64 a7, __int64 a8, __int64 a9, __int64 a10,
               __int64 a11, __int64 a12, __int64 a13, __int64 a14, __int64 a15, __int64 a16) {
        auto original = [&](char* listed, __int64 listed_flags) {
            return ((list_camera_t)entry_list_camera.m_pOriginal)(list, listed, listed_flags, index, a5, a6, a7, a8,
                                                                  a9, a10, a11, a12, a13, a14, a15, a16);
        };
        if (t_pair < 0 || !(flags & 0x300)) return original(camera, flags);
        __int64 module = entry_list_camera.m_target - entry_list_camera.m_offset;
        int slot = (flags & 0x100) ? 0 : 1;
        int source = t_pair == 0 ? slot : (std::min)(2 + slot, Views(module) - 1); // 3 players: the third twice
        char* source_camera = Camera(module, source);
        if (source_camera == nullptr) return original(camera, flags);

        alignas(16) char view[0x398];
        memcpy(view, source_camera, sizeof view);
        float width = *(float*)(view + 0x144), height = *(float*)(view + 0x148);
        *(float*)(view + 0x148) = height * ScreenWidth(module) / width;
        *(int*)(view + 0x220) = slot; // the renderer's per-view tables have two slots
        flags &= ~0x1000;
        if (*(float*)(module + OFFSET_HALO1_PV_SABER_ZOOMED_FOV) > *(float*)(view + 0x14C)) flags |= 0x1000;
        __int64 entry = original(view, flags);
        char* listed = list + 0x10 + (int)entry * 0x3C8 + 0x30;
        *(float*)(listed + 0x148) = height;
        *(float*)(listed + 0x15C) = height / *(float*)(listed + 0x138);
        return entry;
    }

    // ---- the frame

    bool s_quad = false;     // this frame is drawn in quarters
    int s_frame_pair = 0;    // and shows this pair
    int s_frame_views = 0;
    char* s_image = nullptr; // our full-screen image the quadrants are copied into

    // 3 players: the second pair's frames have nobody's second view
    bool NoFourthView(int index) { return s_frame_pair == 1 && index == 1 && s_frame_views < 4; }

    struct CopyRegion {
        char* source;
        char* destination;
        char zero[0x10];
        int x, y;
        __int64 zero2;
        int width, height;
    };

    int TextureWidth(char* texture) { return ((int (*)(char*))(*(void***)texture)[0x18 / 8])(texture); }
    int TextureHeight(char* texture) { return ((int (*)(char*))(*(void***)texture)[0x20 / 8])(texture); }

    void Copy(__int64 module, char* source, char* destination, int x, int y) {
        CopyRegion region{source, destination, {}, x, y, 0, TextureWidth(source), TextureHeight(source)};
        char* context = *(char**)(module + OFFSET_HALO1_PV_SABER_CONTEXT);
        ((void (*)(char*, CopyRegion*))(*(void***)context)[0x190 / 8])(context, &region);
    }

    Halo1Entry(entry_frame, OFFSET_HALO1_PF_SABER_RENDER_FRAME, __int64, frame, void* a, void* b, void* c, void* d) {
        __int64 module = entry_frame.m_target - entry_frame.m_offset;
        s_quad = Quad(module) && (*(unsigned*)RenderList(module) & 1);
        if (!s_quad) return ((frame_t)entry_frame.m_pOriginal)(a, b, c, d);
        s_frame_pair = s_listed_pair;
        s_frame_views = Views(module);
        SwapChildren();
        auto result = ((frame_t)entry_frame.m_pOriginal)(a, b, c, d);
        SwapChildren();
        return result;
    }

    Halo1Entry(entry_composite, OFFSET_HALO1_PF_SABER_COMPOSITE, void, composite, int camera) {
        if (!s_quad) return ((composite_t)entry_composite.m_pOriginal)(camera);
        __int64 module = entry_composite.m_target - entry_composite.m_offset;
        char* view = *(char**)(module + OFFSET_HALO1_PV_SABER_VIEW_IMAGES
                               + 8 * (__int64)*(int*)(module + OFFSET_HALO1_PV_SABER_VIEW_IMAGE_INDEX));
        int width = ScreenWidth(module), height = ScreenHeight(module);
        if (view != nullptr) {
            if (s_image == nullptr || TextureWidth(s_image) != width || TextureHeight(s_image) != height)
                s_image = ((create_child_t)entry_create_child.m_pOriginal)(
                    view, SizedName("ALPHARING_QUAD_IMAGE", width, height).c_str(), width, height,
                    0, *(unsigned short*)(view + 0x7C));
            if (s_image != nullptr && !NoFourthView(camera))
                Copy(module, view, s_image, (camera & 1) * (width / 2), s_frame_pair * (height / 2));
        }
        ((composite_t)entry_composite.m_pOriginal)(camera);
        char* target = *(char**)(module + OFFSET_HALO1_PV_SABER_SCREEN_IMAGE);
        if (camera == 1 && s_image != nullptr && target != nullptr) Copy(module, s_image, target, 0, 0);
    }

    // ---- the classic HUD over the image: its pass draws views 0 and 1 in the screen's halves, then view -1 (the
    // screen's own) over all of it. In quad mode each view gets its quadrant, and views 2 and 3 are drawn before -1.

    thread_local bool t_hud_pass = false;   // in the pass
    thread_local bool t_hud_second = false; // and past view 1

    void SetViewport(__int64 module, int x, int y, int width, int height) {
        char* context = *(char**)(module + OFFSET_HALO1_PV_SABER_CONTEXT);
        ((void (*)(char*, int, int, int, int, float, float))(*(void***)context)[0x120 / 8])(context, x, y, width, height,
                                                                                           0.0f, 1.0f);
    }

    void QuadrantViewport(__int64 module, int view) {
        char* settings = Settings(module);
        int width = *(int*)(settings + 0x10) / 2, height = *(int*)(settings + 0x14) / 2;
        SetViewport(module, (view & 1) * width, (view >> 1) * height, width, height);
    }

    Halo1Entry(entry_hud_pass, OFFSET_HALO1_PF_SABER_CLASSIC_HUD, void, hud_pass) {
        t_hud_pass = s_quad;
        t_hud_second = false;
        ((hud_pass_t)entry_hud_pass.m_pOriginal)();
        t_hud_pass = false;
    }

    Halo1Entry(entry_hud_view, OFFSET_HALO1_PF_HUD_VIEW, void, hud_view, int view) {
        auto original = (hud_view_t)entry_hud_view.m_pOriginal;
        if (t_hand_over) t_view_block = true;
        if (!t_hud_pass) return original(view);
        __int64 module = entry_hud_view.m_target - entry_hud_view.m_offset;
        if (view == 0 || view == 1) {
            QuadrantViewport(module, view);
            t_hud_second = view == 1;
        } else if (view == -1 && t_hud_second) {
            t_hud_second = false;
            for (int extra = 2; extra < s_frame_views; ++extra) {
                QuadrantViewport(module, extra);
                original(extra);
                ((void (*)(int, void*))(module + OFFSET_HALO1_PF_HUD_VIEW_DRAW))(
                    extra, (void*)(module + OFFSET_HALO1_PV_HUD_VIEW_DRAW_STATE));
                ((void (*)())(module + OFFSET_HALO1_PF_HUD_VIEW_DRAW_2))();
                if (!*(int*)(module + OFFSET_HALO1_PV_HUD_VIEW_DRAW_3_SKIP_1)
                    || !*(bool*)(module + OFFSET_HALO1_PV_HUD_VIEW_DRAW_3_SKIP_2))
                    ((void (*)())(module + OFFSET_HALO1_PF_HUD_VIEW_DRAW_3))();
            }
            char* settings = Settings(module);
            SetViewport(module, 0, 0, *(int*)(settings + 0x10), *(int*)(settings + 0x14));
        }
        original(view);
    }

    // ---- each view's screen effects cover its half of the screen: its quadrant instead

    Halo1Entry(entry_overlay, OFFSET_HALO1_PF_SABER_VIEW_OVERLAY, void, view_overlay, void* a, char* camera,
               __int64 index) {
        if (s_quad && NoFourthView((int)index)) return;
        ((view_overlay_t)entry_overlay.m_pOriginal)(a, camera, index);
    }

    extern ::Entry entry_overlay_rect;

    void OverlayRect(char* locals) { // the rect is left r11d, top r9d, right r10d, bottom eax; the index is r8d
        int index = *(int*)(locals - 0x20);
        if (!s_quad || index < 0 || index > 1) return;
        char* settings = Settings(entry_overlay_rect.m_target - entry_overlay_rect.m_offset);
        int width = *(int*)(settings + 0x28) / 2, height = *(int*)(settings + 0x2C) / 2;
        int left = index * width, top = s_frame_pair * height;
        *(__int64*)(locals - 0x38) = left;
        *(__int64*)(locals - 0x28) = top;
        *(__int64*)(locals - 0x30) = left + width;
        *(__int64*)(locals - 0x08) = top + height;
    }
    ::Entry entry_overlay_rect(Halo1EntrySet(), OFFSET_HALO1_SABER_VIEW_OVERLAY_RECT,
                               MidFunctionThunk(&OverlayRect, &entry_overlay_rect.m_pOriginal, nullptr, 0), entry_feature);

    // Which graphics are on screen, as the split-screen hooks last saw them (mcc/splitscreen/LeftRight asks from outside
    // the module's hooks, where it may be unloading): noted while this mode is hooked, Classic otherwise.
    std::atomic<bool> s_anniversary_on_screen = false;
    void NoteGraphics(__int64 module) {
        if (entry_set_split.m_target != 0) s_anniversary_on_screen = AnniversaryShown(module);
    }
    bool ClassicShown() { return !s_anniversary_on_screen; }

    // ---- the module unloading (MCC reloads it for the next game): nothing of ours may point into it

    void Reset() {
        {
            std::lock_guard<std::mutex> lock(s_children_mutex);
            s_children.clear();
        }
        s_image = nullptr;
        s_extra_cameras[0] = s_extra_cameras[1] = nullptr; // the renderer's heap goes with the module
        s_anniversary_on_screen = false;
        s_quarters = false;
        s_quad = false;
        s_sync_quad = false;
        s_pair = s_listed_pair = s_waiting_pair = 0;
    }
    const bool s_reset = (Halo1EntrySet()->on_remove(&Reset), true);
}
