cmake_minimum_required(VERSION 4.0)
project(2D_GameEngine)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(raylib CONFIG REQUIRED)

include_directories(
        src
        src/dependencies
        src/dependencies/include
        src/dependencies/include/extras
)

add_executable(2D_GameEngine
        src/dependencies/include/extras/FA6FreeSolidFontData.h
        src/dependencies/include/extras/IconsFontAwesome6.h
        src/dependencies/include/imconfig.h
        src/dependencies/include/imgui.h
        src/dependencies/include/imgui_impl_raylib.h
        src/dependencies/include/imgui_internal.h
        src/dependencies/include/imstb_rectpack.h
        src/dependencies/include/imstb_textedit.h
        src/dependencies/include/imstb_truetype.h
        src/dependencies/include/rlImGui.h
        src/dependencies/include/rlImGuiColors.h
        src/Entities/EnemyEntity.cpp
        src/Entities/EnemyEntity.h
        src/Managers/EnemyManager.cpp
        src/Managers/EnemyManager.h
        src/Entities/Entity.cpp
        src/Entities/Entity.h
        src/Entities/PlayerEntity.cpp
        src/Entities/PlayerEntity.h
        src/Managers/PlayerManager.cpp
        src/Managers/PlayerManager.h
        src/imgui/imgui.cpp
        src/imgui/imgui_demo.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/rlImGui.cpp
        src/Engine.cpp
        src/Engine.h
        src/Grid.cpp
        src/Grid.h
        src/main.cpp
        src/UI/UI.cpp
        src/UI/UI.h
        src/Entities/TileMap.cpp
        src/Entities/TileMap.h
        src/Scene/Scene.cpp
        src/Scene/Scene.h
        src/Entities/StaticEntity.cpp
        src/Entities/StaticEntity.h
        src/Project/SaveLoad.cpp
        src/Project/SaveLoad.h
        src/Animation/Animator.cpp
        src/Animation/Animator.h
        src/Animation/Animation.h
        src/Animation/AnimationEditor.cpp
        src/Animation/AnimationEditor.h
        src/AssetManagement/AssetManager.cpp
        src/AssetManagement/AssetManager.h
        src/AssetManagement/AssetImporter.cpp
        src/AssetManagement/AssetImporter.h
        src/AssetManagement/AssetDefinition.cpp
        src/AssetManagement/AssetDefinition.h
        src/Managers/LocalizitionManager.cpp
        src/Managers/LocalizitionManager.h
)

target_link_libraries(2D_GameEngine PRIVATE raylib nlohmann_json::nlohmann_json)