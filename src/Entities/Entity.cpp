#include "Entity.h"

Entity::Entity(Vector2 pos, Vector2 size)
    : position(pos), velocity({0,0}), size(size), rotation(0), active(true) {

}

void Entity::Update(float deltaTime) {
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
// =======================================

Rectangle Entity::GetBounds() const {
    return { position.x, position.y, size.x, size.y };
}