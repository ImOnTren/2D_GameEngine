#include "Entity.h"

Entity::Entity(const Vector2 pos, const Vector2 size, const int layer)
    : position(pos), velocity({0,0}), size(size), rotation(0), active(true), layer(layer),  collisionEnabled(true), collisionMask(0xFFFFFFFF) {
}

void Entity::Update(const float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}

void Entity::Draw() {
    DrawRectangleV(position, size, RED); // Placeholder
}

void Entity::OnCollision(Entity* other) {
    // Base entities might just stop; derived can override
    velocity = {0, 0};
}

// =======================================
// Getters and Setters

Vector2 Entity::GetPosition() const {
    return position;
}

void Entity::SetPosition(Vector2 pos) {
    position = pos;
}

Vector2 Entity::GetVelocity() const {
    return velocity;
}

void Entity::SetVelocity(Vector2 vel) {
    velocity = vel;
}

Vector2 Entity::GetSize() const {
    return size;
}

void Entity::SetSize(Vector2 s) {
    size = s;
}

bool Entity::IsActive() const {
    return active;
}

void Entity::SetActive(bool state) {
    active = state;
}

bool Entity::IsCollisionEnabled() const {
    if (collisionEnabled) {
        return true;
    }
    return false;
}

void Entity::SetCollisionEnabled(bool enabled) {
    collisionEnabled = enabled;
}

unsigned int Entity::GetCollisionMask() const {
    return collisionMask;
}

void Entity::SetCollisionMask(const unsigned int& mask) {
    collisionMask = mask;
}

// =======================================

Rectangle Entity::GetBounds() const {
    return { position.x, position.y, size.x, size.y };
}