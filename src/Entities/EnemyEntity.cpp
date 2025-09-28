#include "EnemyEntity.h"
#include "PlayerEntity.h"
#include "raylib.h"

EnemyEntity::EnemyEntity(Grid& grid, int gridX, int gridY)
    : Entity({0,0}, {16,16}), grid(grid), cellX(gridX), cellY(gridY) {
    PlaceOnGrid(gridX, gridY);
}

void EnemyEntity::Update(float deltaTime, const PlayerEntity* player) {
    if (player) {
        Vector2 playerPos = player->GetPlayerPosition();

        // Calculate direction vector towards player
        Vector2 direction = Vector2Subtract(playerPos, position);

        // Normalize the direction vector and apply speed
        if (Vector2Length(direction) > 0) {
            direction = Vector2Normalize(direction);
            Vector2 velocity = Vector2Scale(direction, speed * deltaTime);

            // Update position smoothly
            position = Vector2Add(position, velocity);

            // Update grid coordinates based on new position
            int tileSize = grid.GetTileSize();
            cellX = static_cast<int>(position.x / tileSize);
            cellY = static_cast<int>(position.y / tileSize);
        }
    }
}

void EnemyEntity::Draw() {
    DrawRectangleV(position, size, enemyColor);
}

void EnemyEntity::PlaceOnGrid(int gridX, int gridY) {
    cellX = gridX;
    cellY = gridY;
    int tileSize = grid.GetTileSize();
    position = { static_cast<float>(cellX * tileSize), static_cast<float>(cellY * tileSize) };
    size = { static_cast<float>(tileSize), static_cast<float>(tileSize) };
}