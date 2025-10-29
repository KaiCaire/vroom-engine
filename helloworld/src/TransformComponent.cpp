#include "TransformComponent.h"
#include "GameObject.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

TransformComponent::TransformComponent(GameObject* owner)
    : Component(owner, ComponentType::TRANSFORM),
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
    // Matrices are calculated on-demand with lazy evaluation
}

void TransformComponent::Disable() {
    // Nothing to do on disable
}

void TransformComponent::OnEditor() {
    // Aqui va el laracodigo para ImGui
}

void TransformComponent::SetPosition(const glm::vec3& pos) {
    localPosition = pos;
    MarkAsDirty();
}

void TransformComponent::SetRotation(const glm::quat& rot) {
    localRotation = rot;
    MarkAsDirty();
}

void TransformComponent::SetRotation(const glm::vec3& euler) {
    // Convert euler angles (in degrees) to quaternion
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
    glm::mat4 globalMat = GetGlobalMatrix();
    return glm::vec3(globalMat[3]);
}

glm::quat TransformComponent::GetWorldRotation() const {
    if (!owner || !owner->GetParent()) {
        return localRotation;
    }

    TransformComponent* parentTransform =
        static_cast<TransformComponent*>(owner->GetParent()->GetComponent(ComponentType::TRANSFORM));

    if (parentTransform) {
        return parentTransform->GetWorldRotation() * localRotation;
    }

    return localRotation;
}

glm::vec3 TransformComponent::GetWorldScale() const {
    if (!owner || !owner->GetParent()) {
        return localScale;
    }

    TransformComponent* parentTransform =
        static_cast<TransformComponent*>(
            owner->GetParent()->GetComponent(ComponentType::TRANSFORM));
    if (parentTransform) {
        return parentTransform->GetWorldScale() * localScale;
    }
    return localScale;
}

glm::mat4 TransformComponent::GetLocalMatrix() const {
    if (isDirty) {
        RecalculateMatrices();
    }
    return localMatrix;
}

glm::mat4 TransformComponent::GetGlobalMatrix() const {
    if (isGlobalDirty) {
        RecalculateGlobalMatrixRecursive();
    }
    return globalMatrix;
}

void TransformComponent::MarkAsDirty() {
    isDirty = true;
    isGlobalDirty = true;

    // Mark all children as dirty recursively
    if (owner) {
        for (GameObject* child : owner->GetChildren()) {
            TransformComponent* childTransform =
                static_cast<TransformComponent*>(
                    child->GetComponent(ComponentType::TRANSFORM));
            if (childTransform) {
                childTransform->isGlobalDirty = true;
            }
        }
    }
}

void TransformComponent::RecalculateMatrices() const {
    // Build local matrix from TRS (Translation, Rotation, Scale)
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), localPosition);
    glm::mat4 rotation = glm::mat4_cast(localRotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), localScale);

    localMatrix = translation * rotation * scale;
    isDirty = false;
}

void TransformComponent::RecalculateGlobalMatrixRecursive() const {
    if (isDirty) {
        RecalculateMatrices();
    }

    if (!owner || !owner->GetParent()) {
        // No parent, global matrix = local matrix
        globalMatrix = localMatrix;
    } else {
        // Global = Parent's Global * Local
        TransformComponent* parentTransform =
            static_cast<TransformComponent*>(
                owner->GetParent()->GetComponent(ComponentType::TRANSFORM));
        if (parentTransform) {
            globalMatrix = parentTransform->GetGlobalMatrix() * localMatrix;
        } else {
            globalMatrix = localMatrix;
        }
    }

    isGlobalDirty = false;
}