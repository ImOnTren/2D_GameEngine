// Animator.h
#pragma once

#include "Animation.h"
#include <string>

class Animator {
public:
    Animator() = default;

    void SetAnimationSet(AnimationSet* set);
    AnimationSet* GetAnimationSet() const { return animationSet; }

    // Play animation by name
    void Play(const std::string& animationName, bool forceRestart = false);

    // Play a directional animation and auto-mirror missing horizontal variants.
    // If LEFT is missing, RIGHT can be used flipped; if RIGHT is missing, LEFT can be used flipped.
    void PlayDirectional(const std::string& baseName, AnimationDirection direction, bool forceRestart = false);

    void Stop();
    void Update(float deltaTime);

    Rectangle GetCurrentFrameRect() const;
    Texture2D GetTexture() const;
    bool HasValidTexture() const;

    // Returns true if the current animation should be drawn flipped horizontally
    bool IsFlippedHorizontal() const { return flipHorizontal; }

    bool IsPlaying() const { return playing; }
    bool IsFinished() const { return finished; }
    const std::string& GetCurrentAnimationName() const { return currentAnimationName; }
    AnimationDirection GetCurrentDirection() const { return currentDirection; }

    void Pause() { playing = false; }
    void Resume() { if (currentAnimation) playing = true; }

    void SetSpeed(float speed) { playbackSpeed = speed; }
    float GetSpeed() const { return playbackSpeed; }

    int GetCurrentFrameIndex() const { return currentFrameIndex; }

private:
    AnimationSet* animationSet = nullptr;
    const Animation* currentAnimation = nullptr;
    std::string currentAnimationName;
    AnimationDirection currentDirection = AnimationDirection::NONE;

    int currentFrameIndex = 0;
    float elapsedTime = 0.0f;
    float playbackSpeed = 1.0f;
    bool playing = false;
    bool finished = false;
    bool flipHorizontal = false;

    // Helper to build animation name from base and direction
    std::string BuildAnimationName(const std::string& baseName, AnimationDirection direction) const;
};
