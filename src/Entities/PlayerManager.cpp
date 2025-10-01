#include "PlayerManager.h"
#include "Grid.h"
#include "PlayerEntity.h"

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

bool PlayerManager::PlacePlayer(Vector2 mouseScreen, Camera2D camera,
                               std::vector<std::unique_ptr<Entity>>& entities) {
    if (PlayerExists(entities)) {
        TraceLog(LOG_WARNING, "Player already exists. Remove current player first.");
        return false;
    }

    Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
    int tileSize = grid.GetTileSize();
    int gridX = static_cast<int>(mouseWorld.x / tileSize);
    int gridY = static_cast<int>(mouseWorld.y / tileSize);

    entities.push_back(std::make_unique<PlayerEntity>(grid, gridX, gridY));
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
                return true;
            }
        }
    }
    return false;
}