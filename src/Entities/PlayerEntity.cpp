#include "PlayerEntity.h"

PlayerEntity::PlayerEntity(Grid& g, int gridX, int gridY)
    : Entity({0,0}, {16,16}), grid(g), cellX(gridX), cellY(gridY) {
    PlaceOnGrid(gridX, gridY);
    textureLoaded = LoadPlayerTexture("../src/assets/hugo.png");
}

PlayerEntity::~PlayerEntity() {
    if (textureLoaded) {
        UnloadTexture(playerTexture);
    }
}

bool PlayerEntity::LoadPlayerTexture(const char* path) {
    if (FileExists(path)) {
        playerTexture = LoadTexture(path);
        return true;
    }
    TraceLog(LOG_WARNING, "Failed to load player texture: '%s'", path);
    return false;
}

void PlayerEntity::Update(float deltaTime) {
    // Movement: step one cell at a time
    // TODO: implement smooth movement
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) cellX++;
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) cellX--;
    if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) cellY++;
    if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) cellY--;

    // Clamp to positive values
    cellX = std::max(0, cellX);
    cellY = std::max(0, cellY);

    PlaceOnGrid(cellX, cellY);
}

void PlayerEntity::Draw() {
    if (textureLoaded) {
        DrawTextureV(playerTexture, position, WHITE);
    }
    else {
        DrawRectangleV(position, size, BLUE);
    }
}

void PlayerEntity::PlaceOnGrid(int gridX, int gridY) {
    cellX = gridX;
    cellY = gridY;
    int tileSize = grid.GetTileSize();
    position = { static_cast<float>(cellX * tileSize), static_cast<float>(cellY * tileSize) };
    size = { static_cast<float>(tileSize), static_cast<float>(tileSize) };

}