#pragma once

#include "Entity.h"
#include "Grid.h"
#include "Managers/AssetManager.h"

class StaticEntity : public Entity {
public:
    StaticEntity(Grid& grid,Asset* asset, int gridX = 0, int gridY = 0);
    [[nodiscard]] int GetGridX() const { return cellX; }
    [[nodiscard]] int GetGridY() const { return cellY; }
    [[nodiscard]] Asset* GetAsset() const { return asset; }
    void Update(float deltaTime) override;
    void Draw() override;
    [[nodiscard]] std::unique_ptr<Entity> CreateSnapshot() const override;
    void RestoreFromSnapshot(const Entity* snapshot) override;
private:
    Grid& grid;
    int cellX, cellY;
    Asset* asset;
};