#pragma once
#include <memory>
#include <vector>
#include "raylib.h"
#include "Entity.h"

class Grid;
class PlayerEntity;

class PlayerManager {
public:
    PlayerManager(Grid& grid);

    bool PlayerExists(const std::vector<std::unique_ptr<Entity>>& entities);
    PlayerEntity* FindPlayerEntity(const std::vector<std::unique_ptr<Entity>>& entities);

    bool PlacePlayer(Vector2 mouseScreen, Camera2D camera,
                    std::vector<std::unique_ptr<Entity>>& entities);
    bool RemovePlayer(Vector2 mouseScreen, Camera2D camera,
                     std::vector<std::unique_ptr<Entity>>& entities);

private:
    Grid& grid;
};