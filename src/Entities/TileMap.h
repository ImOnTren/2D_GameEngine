#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

struct TileData {
    std::string tileID;
    int tileIndex;
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

    TileData GetTile(int x, int y);
    void SetTile(int x, int y, TileData tile);
    const auto& GetAllTiles() {return tiles;};

    void RemoveTile(int x, int y);
    bool HasTile(int x, int y);
    void Clear();

private:
    std::unordered_map<uint64_t, TileData> tiles;

    uint64_t PositionToID(int x, int y) {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(y);
    }
};
