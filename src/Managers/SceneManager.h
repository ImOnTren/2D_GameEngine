#pragma once
#include <string>
#include <vector>
#include <memory>

class TileMap;
class Entity;
class PlayerEntity;

class Scene
{
public:
    // --- Construction ---
    Scene(const std::string& id, const std::string& displayName);

    // --- Identity ---
    const std::string& GetId() const;
    const std::string& GetName() const;
    void SetName(const std::string& newName);

    // --- World data ---
    TileMap& GetTileMap();
    const TileMap& GetTileMap() const;

    std::vector<std::unique_ptr<Entity>>& GetEntities();
    const std::vector<std::unique_ptr<Entity>>& GetEntities() const;

    // --- Utilities ---
    void Clear(); // clear tiles + entities for this scene

    // (Optional convenience) – not required right now, but nice:
    PlayerEntity* FindPlayer();
    const PlayerEntity* FindPlayer() const;

private:
    std::string id;          // e.g. "outside", "house_1_interior"
    std::string name;        // e.g. "Outside", "House 1 Interior"

    std::unique_ptr<TileMap> tileMap;
    std::vector<std::unique_ptr<Entity>> editModeEntities;
};
