#include "EnemyManager.h"
#include "Grid.h"
#include "EnemyEntity.h"

EnemyManager::EnemyManager(Grid& grid) : grid(grid) {}

bool EnemyManager::TryPlaceEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<EnemyEntity>>& enemies) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
        int tileSize = grid.GetTileSize();
        int gridX = static_cast<int>(mouseWorld.x / tileSize);
        int gridY = static_cast<int>(mouseWorld.y / tileSize);

        // Limit number of enemies to 10 for performance
        if (enemies.size() >= 10) {
            TraceLog(LOG_WARNING, "Maximum number of enemies reached (10). Cannot place more.");
            return false;
        }

        enemies.push_back(std::make_unique<EnemyEntity>(grid, gridX, gridY)); // Remove camera parameter
        return true; // Successfully placed
    }
    return false; // Not placed
}

bool EnemyManager::TryRemoveEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<EnemyEntity>>& enemies) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
        int tileSize = grid.GetTileSize();
        int clickedGridX = static_cast<int>(mouseWorld.x / tileSize);
        int clickedGridY = static_cast<int>(mouseWorld.y / tileSize);

        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            int enemyGridX = (*it)->GetGridX();
            int enemyGridY = (*it)->GetGridY();

            if (clickedGridX == enemyGridX && clickedGridY == enemyGridY) {
                enemies.erase(it);
                return true; // Successfully removed
            }
        }
    }
    return false; // Not removed
}