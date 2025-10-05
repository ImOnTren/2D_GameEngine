#include "EnemyManager.h"
#include "Grid.h"
#include "EnemyEntity.h"
#include "PlayerEntity.h"
#include "UI/UI.h"

EnemyManager::EnemyManager(Grid& grid) : grid(grid) {}

int EnemyManager::GetEnemyCount(const std::vector<std::unique_ptr<Entity>>& entities) const {
    int count = 0;
    for (auto& entity : entities) {
        if (dynamic_cast<EnemyEntity*>(entity.get())) {
            count++;
        }
    }
    return count;
}

bool EnemyManager::IsCellOccupied(int gridX, int gridY, const std::vector<std::unique_ptr<Entity>>& entities) {
    for (auto& entity : entities) {
        if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
            if (player->GetGridX() == gridX && player->GetGridY() == gridY) {
                return true;
            }
        } else if (auto enemy = dynamic_cast<EnemyEntity*>(entity.get())) {
            if (enemy->GetGridX() == gridX && enemy->GetGridY() == gridY) {
                return true;
            }
        }
    }
    return false;
}

bool EnemyManager::PlaceEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<Entity>>& entities) {
    if (GetEnemyCount(entities) >= 10) {
        UI::SetDebugMessage("[WARNING] Maximum number of enemies reached (10). Cannot place more.");
        return false;
    }

    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(mouseWorld.x / tileSize);
    int gridY = static_cast<int>(mouseWorld.y / tileSize);

    // Check if cell is occupied
    if (IsCellOccupied(gridX, gridY, entities)) {
        UI::SetDebugMessage("[WARNING] Cell is already occupied. Cannot place enemy here.");
        return false;
    }

    entities.push_back(std::make_unique<EnemyEntity>(grid, gridX, gridY));
    UI::SetDebugMessage("[INFO] Enemy placed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
    return true;
}

bool EnemyManager::RemoveEnemy(Vector2 mouseScreen, Camera2D camera,
                              std::vector<std::unique_ptr<Entity>>& entities) {
    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (auto enemy = dynamic_cast<EnemyEntity*>(it->get())) {
            Rectangle enemyBounds = enemy->GetBounds();

            if (CheckCollisionPointRec(mouseWorld, enemyBounds)) {
                entities.erase(it);
                UI::SetDebugMessage("[INFO] Enemy removed at (" + std::to_string(enemy->GetGridX()) + ", " + std::to_string(enemy->GetGridY()) + ")");
                return true;
            }
        }
    }
    return false;
}