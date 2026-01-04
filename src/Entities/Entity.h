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

    bool collisionEnabled;
    unsigned int collisionMask;

public:
    explicit Entity(Vector2 pos = {0,0}, Vector2 size = {16,16});
    virtual ~Entity() = default;

    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void OnCollision(Entity* other);

    // Snapshot methods - to be implemented by derived classes
    [[nodiscard]] virtual std::unique_ptr<Entity> CreateSnapshot() const {
        // Default implementation creates a basic copy
        auto snapshot = std::make_unique<Entity>(*this);
        return snapshot;
    }
    virtual void RestoreFromSnapshot(const Entity* snapshot) {
        // Default implementation copies basic properties
        if (snapshot) {
            position = snapshot->position;
            velocity = snapshot->velocity;
            size = snapshot->size;
            rotation = snapshot->rotation;
            active = snapshot->active;
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

    Rectangle GetBounds() const;
};