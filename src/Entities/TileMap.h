#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

struct TileData {
    std::string tileID;
    int tileIndex;
    bool isSolid = false;
    bool isSceneSwitcher = false;
    std::string targetSceneID;
    int triggerKey = 0;
    int layer = 0;
};

class TileMap {
public:
    TileMap() = default;

    TileMap& operator=(const TileMap& other) {
        if (this != &other) {
            tiles = other.tiles;
        }
        return *this;
    }

    TileData GetTile(int x, int y, int layer = 0);

    std::vector<TileData> *GetTilePtr(int x, int y);
    void SetTile(int x, int y, const TileData& tile, int layer = 0);
    auto& GetAllTiles() const {return tiles;}
    std::vector<TileData> GetTilesAtPosition(int x, int y) const;

    void RemoveTile(int x, int y);
    void RemoveTileFromLayer(int x, int y, int layer);
    bool HasTile(int x, int y, int layer = -1);
    void Clear();

    int GetLayerCount(int x, int y);

private:
    std::unordered_map<uint64_t, std::vector<TileData>> tiles;

    uint64_t PositionToID(const int x, const int y) const {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
    }
};
