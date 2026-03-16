#pragma once
#include "Entity.h"
#include "Grid.h"
#include "PlayerEntity.h"
#include "AssetManagement/AssetManager.h"

class EnemyEntity : public Entity {
public:
    EnemyEntity(Grid& grid, Asset* asset = nullptr, int gridX = 0, int gridY = 0, int layer = 0);
    EnemyEntity(const EnemyEntity& other);

    void Update(float deltaTime) override;
    void Update(float deltaTime, const PlayerEntity* player);
    void Draw() override;
    void OnCollision(Entity* other) override;

    [[nodiscard]] int GetGridX() const { return cellX; }
    [[nodiscard]] int GetGridY() const { return cellY; }
    [[nodiscard]] int GetLayer() const { return layer; }
    void SetLayer(const int newLayer) { layer = newLayer; }

    [[nodiscard]] Asset* GetAsset() const { return asset; }

    void PlaceOnGrid(int gridX, int gridY);

    [[nodiscard]] std::unique_ptr<Entity> CreateSnapshot() const override;
    void RestoreFromSnapshot(const Entity* snapshot) override;

    [[nodiscard]] bool SupportsScaling() const override { return true; }
    [[nodiscard]] float GetScale() const override { return scale; }
    void SetScale(float newScale) override;

private:
    void RecalculateCollisionBox();

    Grid& grid;
    int cellX = 0;
    int cellY = 0;
    int layer = 0;
    Asset* asset = nullptr;
    float speed = 100.0f;

    float scale = 1.0f;
    Vector2 baseSize{16.0f, 16.0f};
};