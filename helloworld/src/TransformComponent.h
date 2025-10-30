#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "Component.h"
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TransformComponent : public Component {

public:
    TransformComponent(GameObject* owner);
    ~TransformComponent() override = default;

    // Component interface pa la lara
    void Enable() override;
    void Update() override;
    void Disable() override;
    void OnEditor() override;

    void SetPosition(const glm::vec3& pos);
    void SetRotation(const glm::quat& rot);
    void SetRotation(const glm::vec3& euler); 
    void SetScale(const glm::vec3& scl);

    glm::vec3 GetPosition() const { return localPosition; }
    glm::quat GetRotation() const { return localRotation; }
    glm::vec3 GetScale() const { return localScale; }
    glm::vec3 GetEulerAngles() const; // Returns in degrees

    // World space transforms
    glm::vec3 GetWorldPosition() const;
    glm::quat GetWorldRotation() const;
    glm::vec3 GetWorldScale() const;

    // Matrix operations
    glm::mat4 GetLocalTransform() const;
    glm::mat4 GetGlobalTransform() const;

    // Mark transform as dirty (needs recalculation)
    void MarkAsDirty();

private:
    void RecalculateMatrices() const;
    void RecalculateGlobalMatrixRecursive() const;

    // Local transform - DEFINED HERE
    glm::vec3 localPosition;
    glm::quat localRotation;
    glm::vec3 localScale;

    // Cached matrices
    mutable glm::mat4 localMatrix;
    mutable glm::mat4 globalMatrix;
    mutable bool isDirty;
    mutable bool isGlobalDirty;
};