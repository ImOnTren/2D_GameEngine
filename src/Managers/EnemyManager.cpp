#include "EnemyManager.h"
#include "Grid.h"
#include "../Entities/EnemyEntity.h"
#include "../Entities/PlayerEntity.h"
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

    if (!currentAsset) {
        UI::SetDebugMessage("[WARNING] No asset selected for enemy placement.");
        return false;
    }

    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(mouseWorld.x / tileSize);
    int gridY = static_cast<int>(mouseWorld.y / tileSize);

    if (!grid.IsValidCell(gridX, gridY)) {
        UI::SetDebugMessage("[WARNING] Cannot place enemy outside grid bounds.");
        return false;
    }

    if (IsCellOccupied(gridX, gridY, entities)) {
        UI::SetDebugMessage("[WARNING] Cell is already occupied. Cannot place enemy here.");
        return false;
    }

    entities.push_back(std::make_unique<EnemyEntity>(grid, currentAsset, gridX, gridY));
    UI::SetDebugMessage("[INFO] Enemy placed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
    return true;
}

bool EnemyManager::RemoveEnemy(Vector2 mouseScreen, Camera2D camera,
                              std::vector<std::unique_ptr<Entity>>& entities) {
    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (auto enemy = dynamic_cast<EnemyEntity*>(it->get())) {
            Rectangle enemyBounds = enemy->GetDrawBounds();

            if (CheckCollisionPointRec(mouseWorld, enemyBounds)) {
                int removedX = enemy->GetGridX();
                int removedY = enemy->GetGridY();

                UI::ClearSelectedEntity();
                entities.erase(it);

                UI::SetDebugMessage("[INFO] Enemy removed at (" +
                    std::to_string(removedX) + ", " +
                    std::to_string(removedY) + ")");
                return true;
            }
        }
    }
    return false;
}
