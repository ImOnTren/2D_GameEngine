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
    // Smooth movement using velocity
    Vector2 input = {0, 0};

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) input.x += 1;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) input.x -= 1;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) input.y += 1;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) input.y -= 1;

    // Normalize diagonal movement
    if (Vector2Length(input) > 0) {
        input = Vector2Normalize(input);
    }

    // Apply velocity
    velocity = Vector2Scale(input, speed);

    // Call base Update to apply velocity to position
    Entity::Update(deltaTime);

    // Update grid coordinates based on new position
    int tileSize = grid.GetTileSize();
    cellX = static_cast<int>(position.x / tileSize);
    cellY = static_cast<int>(position.y / tileSize);

    // Optional: Clamp to positive values if needed
    if (position.x < 0) position.x = 0;
    if (position.y < 0) position.y = 0;
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