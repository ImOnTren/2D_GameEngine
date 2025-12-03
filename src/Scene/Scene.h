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
    Scene(const std::string& id, const std::string& displayName);

    const std::string& GetId() const;
    const std::string& GetName() const;
    void SetName(const std::string& newName);

    TileMap& GetTileMap();
    const TileMap& GetTileMap() const;

    std::vector<std::unique_ptr<Entity>>& GetEntities();
    const std::vector<std::unique_ptr<Entity>>& GetEntities() const;

    void Clear();

    PlayerEntity* FindPlayer();
    const PlayerEntity* FindPlayer() const;

private:
    std::string id;
    std::string name;

    std::unique_ptr<TileMap> tileMap;
    std::vector<std::unique_ptr<Entity>> editModeEntities;
};
