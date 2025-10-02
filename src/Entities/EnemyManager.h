#pragma once
#include <memory>
#include <vector>
#include "raylib.h"
#include "Entity.h"


class Grid;
class EnemyEntity;

class EnemyManager {
public:
    EnemyManager(Grid& grid);

    int GetEnemyCount(const std::vector<std::unique_ptr<Entity>>& entities) const;

    bool IsCellOccupied(int gridX, int gridY, const std::vector<std::unique_ptr<Entity>>& entities);

    bool PlaceEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<Entity>>& entities);
    bool RemoveEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<Entity>>& entities);

private:
    Grid& grid;
};