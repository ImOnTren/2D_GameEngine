#include "UI.h"
#include "Engine.h"

std::vector<std::string> UI::DebugMessages = {"Welcome To Click-Craft Creator"};

void UI::RenderControlPanel(Engine& engine, Grid& grid) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Control panel", NULL,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    RenderGridSizeControls(grid);
    RenderModeControls(engine);
    RenderCameraResolutionControls(engine);

    ImGui::End();
}

void UI::RenderControlConsole() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::Begin("Debug console", NULL,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // Add a clear button
    if (ImGui::Button("Clear Console")) {
        ClearDebugMessages();
    }
    ImGui::SameLine();
    ImGui::Text("Messages: %zu", DebugMessages.size());

    ImGui::Separator();

    // Create a child window for scrolling
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Display all messages (oldest first, newest at bottom)
    for (const auto& message : DebugMessages) {
        ImGui::TextWrapped("%s", message.c_str());
    }

    // Auto-scroll to bottom if new messages were added
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}

void UI::RenderAssetConsole() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - io.DisplaySize.x / 4.0f, io.DisplaySize.y / 4.0f));
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y - io.DisplaySize.y / 4.0f));
    ImGui::Begin("Asset Console", NULL,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::End();
}

void UI::RenderPlayModeWindow(Engine& engine) {
    if (!engine.playModeWindowOpen) return;

    auto& resolutions = engine.GetAvailableResolutions();
    int currentIndex = engine.GetSelectedResolutionIndex();

    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Play Mode", &engine.playModeWindowOpen, ImGuiWindowFlags_NoScrollbar)) {
        ImVec2 contentSize = ImGui::GetContentRegionAvail();

        // Calculate UV coordinates to fix inversion
        ImVec2 uv0 = ImVec2(0, 1);
        ImVec2 uv1 = ImVec2(1, 0);

        // Maintain aspect ratio
        float textureAspect = (float)engine.playModeTexture.texture.width / (float)engine.playModeTexture.texture.height;
        float windowAspect = contentSize.x / contentSize.y;

        ImVec2 displaySize;
        if (textureAspect > windowAspect) {
            displaySize.x = contentSize.x;
            displaySize.y = contentSize.x / textureAspect;
        } else {
            displaySize.y = contentSize.y;
            displaySize.x = contentSize.y * textureAspect;
        }

        // Center the image
        ImVec2 pos = ImGui::GetCursorPos();
        pos.x += (contentSize.x - displaySize.x) * 0.5f;
        pos.y += (contentSize.y - displaySize.y) * 0.5f;
        ImGui::SetCursorPos(pos);

        // Display the texture
        ImGui::Image(
            (ImTextureID)(uintptr_t)engine.playModeTexture.texture.id,
            displaySize,
            uv0,
            uv1,
            ImVec4(1, 1, 1, 1),
            ImVec4(0, 0, 0, 0)
        );

        // Display resolution info
        if (currentIndex >= 0 && currentIndex < resolutions.size()) {
            ImGui::Text("Resolution: %dx%d",
                       resolutions[currentIndex].width,
                       resolutions[currentIndex].height);
        }
    }
    ImGui::End();
}

void UI::RenderGridSizeControls(Grid& grid) {
    grid.RenderSizeAvailability();
}

void UI::RenderModeControls(Engine& engine) {
    ImGui::Separator();
    ImGui::Text("Engine Mode:");

    if (ImGui::RadioButton("Edit Mode", engine.currentMode == Engine::Mode::EDIT)) {
        engine.StopPlayMode();
        engine.ResetTool();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Play Mode", engine.currentMode == Engine::Mode::PLAY)) {
        engine.StartPlayMode();
        engine.ResetTool();
    }

    if (engine.currentMode == Engine::Mode::EDIT) {
        ImGui::Separator();
        ImGui::Text("Edit Tools:");

        // Player placement/removal
        if (ImGui::Button("Place Player", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::PLACING_PLAYER) ?
                Engine::ToolState::NONE : Engine::ToolState::PLACING_PLAYER;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Player", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_PLAYER) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_PLAYER;
        }

        // Enemy placement/removal
        if (ImGui::Button("Place Enemy", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::PLACING_ENEMY) ?
                Engine::ToolState::NONE : Engine::ToolState::PLACING_ENEMY;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Enemy", ImVec2(120, 0))) {
            engine.currentTool = (engine.currentTool == Engine::ToolState::REMOVING_ENEMY) ?
                Engine::ToolState::NONE : Engine::ToolState::REMOVING_ENEMY;
        }

        // Show current tool status
        ImGui::Text("Current Tool: ");
        ImGui::SameLine();
        switch (engine.currentTool) {
            case Engine::ToolState::NONE: ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "None"); break;
            case Engine::ToolState::PLACING_PLAYER: ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Placing Player"); break;
            case Engine::ToolState::REMOVING_PLAYER: ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Removing Player"); break;
            case Engine::ToolState::PLACING_ENEMY: ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Placing Enemy"); break;
            case Engine::ToolState::REMOVING_ENEMY: ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Removing Enemy"); break;
        }

        // Show enemy count
        ImGui::Text("Enemies: %d/10", engine.GetEnemyCount());

        if (ImGui::Button("Clear Tool", ImVec2(120, 0))) {
            engine.ResetTool();
        }
    }
}

void UI::RenderCameraResolutionControls(Engine& engine) {
    ImGui::Separator();
    ImGui::Text("Player Camera Settings:");

    // Use public method to check if player exists
    if (!engine.HasPlayer()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Place a player to see camera preview");
    } else {
        ImGui::Text("Camera preview visible around player");
    }

    // Resolution dropdown using public methods
    ImGui::Text("Resolution:");

    // Get resolutions via public method
    const auto& resolutions = engine.GetAvailableResolutions();
    int currentIndex = engine.GetSelectedResolutionIndex();

    // Create combo box labels
    std::vector<const char*> resolutionLabels;
    for (const auto& res : resolutions) {
        resolutionLabels.push_back(res.name);
    }

    if (ImGui::Combo("##Resolution", &currentIndex, resolutionLabels.data(), resolutionLabels.size())) {
        // Use public setter method
        engine.SetSelectedResolutionIndex(currentIndex);
    }

    // Display current resolution info
    if (currentIndex >= 0 && currentIndex < resolutions.size()) {
        auto& currentRes = resolutions[currentIndex];
        ImGui::Text("Current: %dx%d (%.1f:1)",
                   currentRes.width, currentRes.height,
                   (float)currentRes.width / currentRes.height);

        // Aspect ratio info
        float aspect = (float)currentRes.width / currentRes.height;
        const char* aspectName = "Custom";
        if (abs(aspect - 16.0f/9.0f) < 0.01f) aspectName = "16:9";
        else if (abs(aspect - 4.0f/3.0f) < 0.01f) aspectName = "4:3";
        else if (abs(aspect - 1.0f) < 0.01f) aspectName = "1:1";

        ImGui::Text("Aspect Ratio: %s", aspectName);
    }
}