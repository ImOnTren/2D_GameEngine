// Animator.cpp
#include "Animator.h"

void Animator::SetAnimationSet(AnimationSet* set) {
    animationSet = set;
    Stop();
}

void Animator::Play(const std::string& animationName, bool forceRestart) {
    if (!animationSet) {
        return;
    }
    
    if (!forceRestart && currentAnimationName == animationName && playing) {
        return;
    }
    
    const Animation* anim = animationSet->GetAnimation(animationName);
    if (!anim || anim->frames.empty()) {
        return;
    }
    
    currentAnimation = anim;
    currentAnimationName = animationName;
    currentDirection = anim->direction;
    currentFrameIndex = 0;
    elapsedTime = 0.0f;
    playing = true;
    finished = false;
    flipHorizontal = anim->flipHorizontallyAtRuntime;
}

void Animator::PlayDirectional(const std::string& baseName, AnimationDirection direction, bool forceRestart) {
    if (!animationSet) {
        return;
    }

    bool needsFlip = false;
    std::string animName = BuildAnimationName(baseName, direction);

    if (direction == AnimationDirection::LEFT || direction == AnimationDirection::RIGHT) {
        const AnimationDirection oppositeDirection =
            (direction == AnimationDirection::LEFT) ? AnimationDirection::RIGHT : AnimationDirection::LEFT;
        const std::string oppositeName = BuildAnimationName(baseName, oppositeDirection);

        const Animation* directAnim = animationSet->GetAnimation(animName);
        if (directAnim && !directAnim->frames.empty()) {
            needsFlip = directAnim->flipHorizontallyAtRuntime;
        } else {
            const Animation* oppositeAnim = animationSet->GetAnimation(oppositeName);
            if (oppositeAnim && !oppositeAnim->frames.empty()) {
                animName = oppositeName;
                // Mirror opposite direction while preserving any existing runtime-flip intent.
                needsFlip = !oppositeAnim->flipHorizontallyAtRuntime;
            }
        }
    }
    
    // Check if already playing this animation with same flip state
    if (!forceRestart && currentAnimationName == animName && 
        flipHorizontal == needsFlip && playing) {
        return;
    }
    
    const Animation* anim = animationSet->GetAnimation(animName);
    if (!anim || anim->frames.empty()) {
        return;
    }
    
    currentAnimation = anim;
    currentAnimationName = animName;
    currentDirection = direction;  // Store the requested direction, not the actual
    currentFrameIndex = 0;
    elapsedTime = 0.0f;
    playing = true;
    finished = false;
    flipHorizontal = needsFlip;
}

std::string Animator::BuildAnimationName(const std::string& baseName, AnimationDirection direction) const {
    switch (direction) {
        case AnimationDirection::DOWN:  return baseName + "_down";
        case AnimationDirection::UP:    return baseName + "_up";
        case AnimationDirection::RIGHT: return baseName + "_right";
        case AnimationDirection::LEFT:  return baseName + "_left";
        case AnimationDirection::NONE:  return baseName;
    }
    return baseName;
}

void Animator::Stop() {
    playing = false;
    finished = false;
    currentFrameIndex = 0;
    elapsedTime = 0.0f;
    currentAnimation = nullptr;
    currentAnimationName.clear();
    currentDirection = AnimationDirection::NONE;
    flipHorizontal = false;
}

void Animator::Update(float deltaTime) {
    if (!playing || !currentAnimation || currentAnimation->frames.empty()) {
        return;
    }
    
    elapsedTime += deltaTime * playbackSpeed;

    while (true) {
        const AnimationFrame& currentFrame = currentAnimation->frames[currentFrameIndex];
        if (elapsedTime < currentFrame.duration) {
            break;
        }

        elapsedTime -= currentFrame.duration;
        currentFrameIndex++;
        
        if (currentFrameIndex >= static_cast<int>(currentAnimation->frames.size())) {
            if (currentAnimation->loop) {
                currentFrameIndex = 0;
            } else {
                currentFrameIndex = static_cast<int>(currentAnimation->frames.size()) - 1;
                playing = false;
                finished = true;
                return;
            }
        }
    }
}

Rectangle Animator::GetCurrentFrameRect() const {
    if (!currentAnimation || currentAnimation->frames.empty()) {
        return {0, 0, 0, 0};
    }
    
    int frameIdx = currentFrameIndex;
    if (frameIdx < 0) frameIdx = 0;
    if (frameIdx >= static_cast<int>(currentAnimation->frames.size())) {
        frameIdx = static_cast<int>(currentAnimation->frames.size()) - 1;
    }
    
    return currentAnimation->frames[frameIdx].sourceRect;
}

Texture2D Animator::GetTexture() const {
    if (currentAnimation && currentAnimation->hasSourceTexture && currentAnimation->sourceTexture.id != 0) {
        return currentAnimation->sourceTexture;
    }
    if (animationSet && animationSet->textureLoaded) {
        return animationSet->texture;
    }
    return Texture2D{0};
}

bool Animator::HasValidTexture() const {
    return animationSet && animationSet->textureLoaded && animationSet->texture.id != 0;
}
