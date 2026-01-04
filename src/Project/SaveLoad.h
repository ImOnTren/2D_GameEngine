#pragma once

#include <string>

class Engine;
class TileMap;
class Entity;

namespace SaveLoad {
    bool SaveProject(Engine& engine, const std::string& filename);
    bool LoadProject(Engine& engine, const std::string& filename);
}