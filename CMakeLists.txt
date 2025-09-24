cmake_minimum_required(VERSION 4.0)
project(2D_GameEngine)

set(CMAKE_CXX_STANDARD 14)

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
        src/Entities/EnemyManager.cpp
        src/Entities/EnemyManager.h
        src/Entities/Entity.cpp
        src/Entities/Entity.h
        src/Entities/PlayerEntity.cpp
        src/Entities/PlayerEntity.h
        src/Entities/PlayerManager.cpp
        src/Entities/PlayerManager.h
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
)

target_link_libraries(2D_GameEngine PRIVATE raylib)