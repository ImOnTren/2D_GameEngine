#pragma once

#include "Entity.h"
#include "Grid.h"
#include "AssetManagement/AssetManager.h"
#include "Animation/Animator.h"

class StaticEntity : public Entity {
public:
    StaticEntity(Grid& grid,Asset* asset, int gridX = 0, int gridY = 0, int layer = 0);
    [[nodiscard]] int GetGridX() const { return cellX; }
    [[nodiscard]] int GetGridY() const { return cellY; }
    [[nodiscard]] int GetLayer() const { return layer; }
    void SetLayer(const int newLayer) { layer = newLayer; }
    [[nodiscard]] Asset* GetAsset() const { return asset; }
    [[nodiscard]] float GetScale() const { return scale; }
    void SetScale(float newScale);
    void Update(float deltaTime) override;
    void Draw() override;
    [[nodiscard]] std::unique_ptr<Entity> CreateSnapshot() const override;
    void RestoreFromSnapshot(const Entity* snapshot) override;
private:
    Animator animator;
    Grid& grid;
    int cellX, cellY;
    int layer = 0;
    Asset* asset;
    float scale = 1.0f;
    Vector2 baseSize;
};