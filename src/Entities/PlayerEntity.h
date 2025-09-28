#pragma once
#include "Entity.h"
#include "Grid.h"

class PlayerEntity : public Entity {
public:
    PlayerEntity(Grid& grid, int gridX = 0, int gridY = 0);
    PlayerEntity(const PlayerEntity& other); // Copy constructor
    ~PlayerEntity();

    void Update(float deltaTime) override;
    void Draw();
    void PlaceOnGrid(int gridX, int gridY);

    // Snapshot methods
    std::unique_ptr<Entity> CreateSnapshot() const override;
    void RestoreFromSnapshot(const Entity* snapshot) override;

    int GetGridX() const { return cellX; }
    int GetGridY() const { return cellY; }
    Vector2 GetPlayerPosition() const { return position; }

    bool LoadPlayerTexture(const char* path);

private:
    Grid& grid;
    int cellX, cellY;
    Texture2D playerTexture;
    bool textureLoaded = false;
    float speed = 100.0f;
};
