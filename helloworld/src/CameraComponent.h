#pragma once
#include "Component.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h" // For Frustum and Plane structs

class TransformComponent;

class CameraComponent : public Component {
public:
    CameraComponent(std::shared_ptr<GameObject> owner);
    ~CameraComponent() override = default;

    // Component interface
    void Enable() override;
    void Update() override;
    void Disable() override;
    void OnEditor() override;

    // Camera settings
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    float aspectRatio = 1.777f; // Default 16:9

    bool isMainCamera = true; // Flag for the Camera used in the Game Viewport

    // Matrices
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // Frustum for culling
    Frustum frustum;
    void ExtractFrustumPlanes(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);

private:
    void NormalizePlane(Plane& plane);
    mutable glm::mat4 viewMatrix;
    mutable glm::mat4 projectionMatrix;
};