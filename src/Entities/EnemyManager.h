// EnemyManager.h
#pragma once
#include <memory>
#include <vector>
#include "raylib.h"

class Grid;
class EnemyEntity;

class EnemyManager {
public:
    EnemyManager(Grid& grid); // Remove camera parameter
    ~EnemyManager() = default;
    bool TryPlaceEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<EnemyEntity>>& enemies);
    bool TryRemoveEnemy(Vector2 mouseScreen, Camera2D camera, std::vector<std::unique_ptr<EnemyEntity>>& enemies);
private:
    Grid& grid;
};