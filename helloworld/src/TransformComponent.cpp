#include "TransformComponent.h"
#include "GameObject.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

TransformComponent::TransformComponent(std::shared_ptr<GameObject> ownerPtr)
    : Component(ownerPtr, ComponentType::TRANSFORM),
    localPosition(0.0f, 0.0f, 0.0f),
    localRotation(1.0f, 0.0f, 0.0f, 0.0f), // Identity quaternion
    localScale(1.0f, 1.0f, 1.0f),
    localMatrix(1.0f),
    globalMatrix(1.0f),
    isDirty(true),
    isGlobalDirty(true) {
}

void TransformComponent::Enable() {
    MarkAsDirty();
}

void TransformComponent::Update() {
    // Transform doesn't need per-frame updates
    // Matrices are calculated on-demand
}

void TransformComponent::Disable() {}

void TransformComponent::OnEditor() {}

void TransformComponent::SetPosition(const glm::vec3& pos) {
    localPosition = pos;
    MarkAsDirty();
}

void TransformComponent::SetRotation(const glm::quat& rot) {
    localRotation = rot;
    MarkAsDirty();
}

void TransformComponent::SetRotation(const glm::vec3& euler) {
    glm::vec3 radians = glm::radians(euler);
    localRotation = glm::quat(radians);
    MarkAsDirty();
}

void TransformComponent::SetScale(const glm::vec3& scl) {
    localScale = scl;
    MarkAsDirty();
}

glm::vec3 TransformComponent::GetEulerAngles() const {
    return glm::degrees(glm::eulerAngles(localRotation));
}

glm::vec3 TransformComponent::GetWorldPosition() const {
    return glm::vec3(GetGlobalTransform()[3]);
}

glm::quat TransformComponent::GetWorldRotation() const {
    if (auto go = owner.lock()) {
        if (auto parent = go->GetParent()) {
            auto parentTransform = std::dynamic_pointer_cast<TransformComponent>(
                parent->GetComponent(ComponentType::TRANSFORM)
            );
            if (parentTransform) {
                return parentTransform->GetWorldRotation() * localRotation;
            }
        }
    }
    return localRotation;
}

glm::vec3 TransformComponent::GetWorldScale() const {
    if (auto go = owner.lock()) {
        if (auto parent = go->GetParent()) {
            auto parentTransform = std::dynamic_pointer_cast<TransformComponent>(
                parent->GetComponent(ComponentType::TRANSFORM)
            );
            if (parentTransform) {
                return parentTransform->GetWorldScale() * localScale;
            }
        }
    }
    return localScale;
}

glm::mat4 TransformComponent::GetLocalTransform() const {
    if (isDirty) RecalculateMatrices();
    return localMatrix;
}

glm::mat4 TransformComponent::GetGlobalTransform() const {
    if (isGlobalDirty) RecalculateGlobalMatrixRecursive();
    return globalMatrix;
}

void TransformComponent::MarkAsDirty() {
    isDirty = true;
    isGlobalDirty = true;

    if (auto go = owner.lock()) {
        for (auto& child : go->GetChildren()) {
            auto childTransform = std::dynamic_pointer_cast<TransformComponent>(
                child->GetComponent(ComponentType::TRANSFORM)
            );
            if (childTransform) {
                childTransform->isGlobalDirty = true;
            }
        }
    }
}

void TransformComponent::RecalculateMatrices() const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), localPosition);
    glm::mat4 rotation = glm::mat4_cast(localRotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), localScale);

    localMatrix = translation * rotation * scale;
    isDirty = false;
}

void TransformComponent::RecalculateGlobalMatrixRecursive() const {
    if (isDirty) RecalculateMatrices();

    if (auto go = owner.lock()) {
        if (auto parent = go->GetParent()) {
            auto parentTransform = std::dynamic_pointer_cast<TransformComponent>(
                parent->GetComponent(ComponentType::TRANSFORM)
            );
            if (parentTransform) {
                globalMatrix = parentTransform->GetGlobalTransform() * localMatrix;
            }
            else {
                globalMatrix = localMatrix;
            }
        }
        else {
            globalMatrix = localMatrix;
        }
    }
    else {
        globalMatrix = localMatrix;
    }

    isGlobalDirty = false;
}