#pragma once
#include "raylib.h"
#include <string>

class Entity {
protected:
    Vector2 position;       // World position
    Vector2 velocity;              // Movement per update
    Vector2 size;                  // Width & height
    float rotation;                // For rotation-based movement/animation
    bool active;                   // Is entity alive/active in the game world
public:
    Entity(Vector2 pos = {0,0}, Vector2 size = {16,16});
    virtual ~Entity() = default;

    virtual void Update(float deltaTime);
    virtual void Draw();
    virtual void OnCollision(Entity* other);

    Vector2 GetPosition() const;
    void SetPosition(Vector2 pos);
    Vector2 GetVelocity() const;
    void SetVelocity(Vector2 vel);
    Vector2 GetSize() const;
    void SetSize(Vector2 s);
    bool IsActive() const;
    void SetActive(bool state);

    Rectangle GetBounds() const; // For collision
};