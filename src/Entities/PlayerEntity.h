#pragma once
#include "Entity.h"
#include "Grid.h"
#include "AssetManagement/AssetManager.h"

class PlayerEntity : public Entity {
public:
    PlayerEntity(Grid& grid, Asset* asset = nullptr, int gridX = 0, int gridY = 0, int layer = 0);
    PlayerEntity(const PlayerEntity& other);
    ~PlayerEntity() override;

    void Update(float deltaTime) override;
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

    Vector2 GetPreviousPosition() const { return previousPosition; }
    int GetPreviousGridX() const { return previousGridX; }
    int GetPreviousGridY() const { return previousGridY; }
    void RestorePreviousTransform();
    void SetWorldPositionAndUpdateGrid(Vector2 worldPosition);

private:
    void RecalculateCollisionBox();

    Grid& grid;
    int cellX = 0;
    int cellY = 0;
    int layer = 0;
    Asset* asset = nullptr;

    Vector2 previousPosition{0.0f, 0.0f};
    int previousGridX = 0;
    int previousGridY = 0;

    float speed = 180.0f;

    Texture2D playerTexture{};
    bool textureLoaded = false;

    float scale = 1.0f;
    Vector2 baseSize{16.0f, 16.0f};
};