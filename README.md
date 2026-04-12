# Click-Craft Creator

**Click-Craft Creator** is a modular 2D game engine in C++ built for creating games without coding.

## Key Features
- Edit Mode and Play Mode with snapshot rollback.
- Multi-layer tile map editing and entity placement.
- Asset importer for PNGs, spritesheets, and tilesets.
- Asset definitions persisted in `assets.json`.

## Technical Stack
- C++20
- raylib
- Dear ImGui + rlImGui
- nlohmann/json
- CMake

## Getting Started (Windows)

### Option A (Recommended): one command, auto-installs CMake if missing
1. Download this repository as ZIP and extract it.
2. Open `cmd` in the extracted project folder.
3. Run:

```bat
setup_windows.bat
```

4. Open `build\Release\2D_GameEngine.exe`.

### Option B: manual CMake commands
1. Download this repository as ZIP and extract it.
2. Open `cmd` in the extracted project folder.
3. Run:

```bat
cmake -S . -B build
cmake --build build --config Release
```

4. Open `build\Release\2D_GameEngine.exe`.

## Notes
- Build copies `src/assets` next to the executable automatically.
- Keep the generated `assets` folder beside `2D_GameEngine.exe`.
- Dependencies (`raylib`, `nlohmann/json`) are fetched by CMake during configure.

## Development Roadmap
- [ ] Interactive tutorial
- [ ] Full English/Slovak localization toggle
- [ ] Animated tiles
- [ ] Multi-scene polish and demo project
