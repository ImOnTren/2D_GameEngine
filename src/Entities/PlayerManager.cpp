#include "PlayerManager.h"
#include "Grid.h"
#include "PlayerEntity.h"

PlayerManager::PlayerManager(Grid& grid) : grid(grid) {}

bool PlayerManager::TryPlacePlayer(Vector2 mouseScreen, Camera2D camera, std::unique_ptr<PlayerEntity>& player) {
    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(mouseWorld.x / tileSize);
    int gridY = static_cast<int>(mouseWorld.y / tileSize);

    player = std::make_unique<PlayerEntity>(grid, gridX, gridY);
    return true; // Successfully created
}