#pragma once

#include <Windows.h>
#include <Xinput.h>

#include <functional>
#include <string>
#include <vector>

// In-game spawn menu: vehicles, weapons, equipment and AI characters in front of a player.
// Each supported game implements a Backend; its functions run on the game thread.
namespace MCC::Spawn {
    enum Category : int { Vehicles, Weapons, Equipment, Characters, kCategoryCount };
    enum Team : int { TeamDefault, TeamAlly, TeamEnemy, kTeamCount };
    constexpr int kMaxPlayers = 4;

    // Local players in the running game (1 without split-screen).
    int LocalPlayerCount();

    struct Item {
        int id;           // backend-defined (a tag index or a palette index)
        std::string name; // display name
    };

    struct Backend {
        // Fill `out` with what the loaded map can spawn in `category`.
        void (*list)(Category category, std::vector<Item>& out);
        // Spawn `id` in front of local player `player` (0-3); returns a short status message.
        std::string (*spawn)(Category category, int id, int player, Team team);
    };

    void RegisterBackend(int game, const Backend* backend);

    // Replace `player`'s status line (for results known only after spawn returns).
    void ReportStatus(int player, const std::string& status);

    // Desktop overlay window (F4).
    void ImGuiContext();

    // Per-player controller menus, drawn in each player's split-screen view.
    // Called from the game's input poll with local player `player`'s pad; true while that
    // player's menu is open, when the menu owns the pad and the game must not see it.
    bool HandlePlayerInput(int player, const XINPUT_GAMEPAD& pad);
    bool AnyPlayerMenuOpen();
    // Draws open player menus and acts on their input; call inside an ImGui frame.
    void RenderPlayerMenus();
}

// Shared by the two menus (implementation detail of mcc/spawn).
namespace MCC::Spawn::Catalog {
    // The game whose catalog applies right now, or -1 outside a mission / without a backend.
    int CurrentGame();
    // Ask the game thread for a new catalog of `game` (automatic when the map changes).
    void Refresh(int game, bool force);
    // Calls `f` with `game`'s items of `category` under the catalog lock, or nullptr while
    // the catalog is loading.
    void Read(int game, Category category, const std::function<void(const std::vector<Item>*)>& f);
    int Size(int game, Category category);
    bool Get(int game, Category category, int index, Item& item);
    std::string Status(int player);
    // Spawns `item` in front of `player`.
    void Spawn(Category category, const Item& item, int player, Team team);
}
