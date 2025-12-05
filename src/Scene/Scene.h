#pragma once
#include <string>
#include <vector>
#include <memory>
#include "raylib.h"

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

    std::vector<std::unique_ptr<Entity>>& GetEditModeEntities();
    std::vector<std::unique_ptr<Entity>>& GetPlayModeSnapshots();

    const Rectangle& GetEditModeCameraArea() const;
    void SetEditModeCameraArea(const Rectangle& newArea);
    const Rectangle& GetPlayModeCameraArea() const;
    void SetPlayModeCameraArea(const Rectangle& newArea);

    void Clear();

    PlayerEntity* FindPlayer();
    const PlayerEntity* FindPlayer() const;

private:
    std::string id;
    std::string name;

    std::unique_ptr<TileMap> tileMap;
    std::vector<std::unique_ptr<Entity>> editModeEntities;
    // Play mode uses snapshots - much more memory efficient
    std::vector<std::unique_ptr<Entity>> playModeSnapshots;

    // Separate camera areas for edit mode and play mode
    Rectangle editModeCameraArea = {0, 0, 0, 0};  // Frozen during play mode
    Rectangle playModeCameraArea = {0, 0, 0, 0};  // Updated during play mode
};
