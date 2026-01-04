#include "TileMap.h"

void TileMap::RemoveTile(int x, int y){
    tiles.erase(PositionToID(x, y));
}

void TileMap::Clear(){
    tiles.clear();
}

bool TileMap::HasTile(int x, int y){
    return tiles.find(PositionToID(x, y)) != tiles.end();
}

void TileMap::SetTile(int x, int y, TileData tile){
    tiles[PositionToID(x, y)] = tile;
}

TileData TileMap::GetTile(int x, int y){
    return tiles[PositionToID(x, y)];
}

TileData* TileMap::GetTilePtr(int x, int y) {
    auto it = tiles.find(PositionToID(x, y));
    if (it != tiles.end()) {
        return &it->second;
    }
    return nullptr;
}
