#include "PlayerManager.h"
#include "Grid.h"
#include "PlayerEntity.h"

PlayerManager::PlayerManager(Grid& grid) : grid(grid) {}

bool PlayerManager::TryPlacePlayer(Vector2 mouseScreen, Camera2D camera, std::unique_ptr<PlayerEntity>& player) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
        int tileSize = grid.GetTileSize();
        int gridX = static_cast<int>(mouseWorld.x / tileSize);
        int gridY = static_cast<int>(mouseWorld.y / tileSize);

        if (!player) {
            player = std::make_unique<PlayerEntity>(grid, gridX, gridY); // Remove camera parameter
            return true; // Successfully placed
        }
    }
    return false; // Not placed
}

bool PlayerManager::TryRemovePlayer(Vector2 mouseScreen, Camera2D camera, std::unique_ptr<PlayerEntity>& player) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player) {
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
        int tileSize = grid.GetTileSize();
        int clickedGridX = static_cast<int>(mouseWorld.x / tileSize);
        int clickedGridY = static_cast<int>(mouseWorld.y / tileSize);

        int playerGridX = player->GetGridX();
        int playerGridY = player->GetGridY();

        if (clickedGridX == playerGridX && clickedGridY == playerGridY) {
            player.reset();
            return true; // Successfully removed
        }
    }
    return false; // Not removed
}