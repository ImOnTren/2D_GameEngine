#include "PlayerManager.h"
#include "EnemyEntity.h"
#include "Grid.h"
#include "PlayerEntity.h"
#include "UI/UI.h"

PlayerManager::PlayerManager(Grid& grid) : grid(grid) {}

bool PlayerManager::PlayerExists(const std::vector<std::unique_ptr<Entity>>& entities) {
    return FindPlayerEntity(entities) != nullptr;
}

PlayerEntity* PlayerManager::FindPlayerEntity(const std::vector<std::unique_ptr<Entity>>& entities) {
    for (auto& entity : entities) {
        if (auto player = dynamic_cast<PlayerEntity*>(entity.get())) {
            return player;
        }
    }
    return nullptr;
}

bool PlayerManager::IsCellOccupied(int gridX, int gridY, const std::vector<std::unique_ptr<Entity>>& entities) {
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

bool PlayerManager::PlacePlayer(Vector2 mouseScreen, Camera2D camera,
                               std::vector<std::unique_ptr<Entity>>& entities) {
    if (PlayerExists(entities)) {
        UI::SetDebugMessage("[WARNING] Player already exists. Remove current player first.");
        return false;
    }

    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(mouseWorld.x / tileSize);
    int gridY = static_cast<int>(mouseWorld.y / tileSize);

    // Check if cell is occupied
    if (IsCellOccupied(gridX, gridY, entities)) {
        UI::SetDebugMessage("[WARNING] Cell is already occupied. Cannot place player here.");
        return false;
    }

    entities.push_back(std::make_unique<PlayerEntity>(grid, gridX, gridY));
    UI::SetDebugMessage("[INFO] Player placed at (" + std::to_string(gridX) + ", " + std::to_string(gridY) + ")");
    return true;
}

bool PlayerManager::RemovePlayer(Vector2 mouseScreen, Camera2D camera,
                                std::vector<std::unique_ptr<Entity>>& entities) {
    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (auto player = dynamic_cast<PlayerEntity*>(it->get())) {
            Rectangle playerBounds = player->GetBounds();

            if (CheckCollisionPointRec(mouseWorld, playerBounds)) {
                entities.erase(it);
                UI::SetDebugMessage("[INFO] Player removed");
                return true;
            }
        }
    }
    return false;
}