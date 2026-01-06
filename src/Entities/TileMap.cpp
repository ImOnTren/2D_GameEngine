#include "TileMap.h"

void TileMap::RemoveTile(int x, int y){
    tiles.erase(PositionToID(x, y));
}

void TileMap::Clear(){
    tiles.clear();
}

bool TileMap::HasTile(int x, int y, int layer){
    uint64_t id = PositionToID(x, y);
    auto it = tiles.find(id);
    if (it != tiles.end()) {
        if (layer == -1) {
            return true;
        }
        auto& tileLayers = it->second;
        if (layer >= 0 && layer < static_cast<int>(tileLayers.size())) {
            return true;
        }
    }
    return false;
}

void TileMap::SetTile(int x, int y, const TileData& tile, int layer){
    uint64_t id = PositionToID(x, y);
    auto& tileLayers = tiles[id];
    // Ensure the vector is large enough
    if (tileLayers.size() <= layer) {
        tileLayers.resize(layer + 1);
    }
    tileLayers[layer] = tile;
}

TileData TileMap::GetTile(int x, int y, int layer){
    uint64_t id = PositionToID(x, y);
    auto it = tiles.find(id);
    if (it != tiles.end()) {
        auto& tileLayers = it->second;
        if (layer >= 0 && layer < static_cast<int>(tileLayers.size())) {
            return tileLayers[layer];
        }
    }
    return TileData{};
}

std::vector<TileData> TileMap::GetTilesAtPosition(int x, int y) const {
    uint64_t id = PositionToID(x, y);
    std::vector<TileData> empty;
    auto it = tiles.find(id);
    if (it != tiles.end()) {
        return it->second;
    }
    return empty;
}


std::vector<TileData> *TileMap::GetTilePtr(int x, int y) {
    auto it = tiles.find(PositionToID(x, y));
    if (it != tiles.end()) {
        return &it->second;
    }
    return nullptr;
}

void TileMap::RemoveTileFromLayer(int x, int y, int layer) {
    std::vector<TileData> tileVec = GetTilesAtPosition(x, y);
    if (layer >= 0 && layer < static_cast<int>(tileVec.size())) {
        tileVec.erase(tileVec.begin() + layer);
        if (tileVec.empty()) {
            RemoveTile(x, y);
        } else {
            tiles[PositionToID(x, y)] = tileVec;
        }
    }
}

int TileMap::GetLayerCount(int x, int y) {
    uint64_t id = PositionToID(x, y);
    auto it = tiles.find(id);
    if (it != tiles.end()) {
        return static_cast<int>(it->second.size());
    }
    return 0;
}