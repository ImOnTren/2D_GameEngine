// EnemyEntity.h
#pragma once
#include "Entity.h"
#include "Grid.h"

class PlayerEntity;

class EnemyEntity : public Entity {
public:
    EnemyEntity(Grid& grid, int gridX = 0, int gridY = 0); // Remove camera parameter

    void Update(float deltaTime, const PlayerEntity* player);
    void Draw();
    void PlaceOnGrid(int gridX, int gridY);

    int GetGridX() const { return cellX; }
    int GetGridY() const { return cellY; }

private:
    Grid& grid;
    int cellX, cellY;
    Color enemyColor = RED;
};