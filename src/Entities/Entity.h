#pragma once
#include "raylib.h"
#include <string>
#include <memory>

class Entity {
protected:
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    float rotation;
    bool active;
    int layer = 0;

    bool collisionEnabled;
    unsigned int collisionMask;

    Vector2 collisionOffset{0.0f, 0.0f};
    Vector2 collisionSize{16.0f, 16.0f};

public:
    explicit Entity(Vector2 pos = {0,0}, Vector2 size = {16,16}, int layer = 0);
    virtual ~Entity() = default;

    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void OnCollision(Entity* other);

    [[nodiscard]] virtual std::unique_ptr<Entity> CreateSnapshot() const {
        auto snapshot = std::make_unique<Entity>(*this);
        return snapshot;
    }

    virtual void RestoreFromSnapshot(const Entity* snapshot) {
        if (snapshot) {
            position = snapshot->position;
            velocity = snapshot->velocity;
            size = snapshot->size;
            rotation = snapshot->rotation;
            active = snapshot->active;
            layer = snapshot->layer;
            collisionEnabled = snapshot->collisionEnabled;
            collisionMask = snapshot->collisionMask;
            collisionOffset = snapshot->collisionOffset;
            collisionSize = snapshot->collisionSize;
        }
    }

    [[nodiscard]] Vector2 GetPosition() const;
    void SetPosition(Vector2 pos);

    [[nodiscard]] Vector2 GetVelocity() const;
    void SetVelocity(Vector2 vel);

    [[nodiscard]] Vector2 GetSize() const;
    void SetSize(Vector2 s);

    [[nodiscard]] bool IsActive() const;
    void SetActive(bool state);

    [[nodiscard]] bool IsCollisionEnabled() const;
    void SetCollisionEnabled(bool enabled);

    [[nodiscard]] unsigned int GetCollisionMask() const;
    void SetCollisionMask(const unsigned int& mask);

    [[nodiscard]] int GetEntityLayer() const { return layer; }
    void SetEntityLayer(const int newLayer) { layer = newLayer; }

    void SetCollisionBox(Vector2 offset, Vector2 hitboxSize) {
        collisionOffset = offset;
        collisionSize = hitboxSize;
    }

    [[nodiscard]] Vector2 GetCollisionOffset() const { return collisionOffset; }
    [[nodiscard]] Vector2 GetCollisionSize() const { return collisionSize; }

    [[nodiscard]] virtual bool SupportsScaling() const { return false; }
    [[nodiscard]] virtual float GetScale() const { return 1.0f; }
    virtual void SetScale(float) {}

    [[nodiscard]] Rectangle GetBounds() const;
    [[nodiscard]] Rectangle GetDrawBounds() const;
};