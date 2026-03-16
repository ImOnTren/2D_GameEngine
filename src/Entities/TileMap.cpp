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

void TileMap::SetTile(const int x, const int y, const TileData& tile, const int layer){
    uint64_t id = PositionToID(x, y);
    TileData tileWithCorrectLayer = tile;
    tileWithCorrectLayer.layer = layer;
    tiles[id][layer] = tileWithCorrectLayer;
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
    std::vector<TileData> result;
    auto it = tiles.find(id);
    if (it != tiles.end()) {
        for (auto const& [layerIndex, tile] : it->second) {
            result.push_back(tile);
        }
    }
    return result;
}


std::map<int, TileData> *TileMap::GetTilePtr(int x, int y) {
    auto it = tiles.find(PositionToID(x, y));
    if (it != tiles.end()) {
        return &it->second;
    }
    return nullptr;
}

void TileMap::RemoveTileFromLayer(int x, int y, int layer) {
    uint64_t id = PositionToID(x, y);
    auto it = tiles.find(id);
    if (it != tiles.end()) {
        it->second.erase(layer); // Remove only the specific layer
        if (it->second.empty()) {
            tiles.erase(it); // Remove the coordinate entirely if no layers left
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