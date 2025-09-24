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

        if (playerPos.x > position.x) cellX++;
        if (playerPos.x < position.x) cellX--;
        if (playerPos.y > position.y) cellY++;
        if (playerPos.y < position.y) cellY--;

        // Update grid position based on world position
        PlaceOnGrid(cellX, cellY);
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