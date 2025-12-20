#pragma once
#include "Component.h"
#include "glm/glm.hpp"

class CameraComponent : public Component {
public:
    CameraComponent(std::shared_ptr<GameObject> owner);
    virtual ~CameraComponent() = default;

    void Update() override;

    //settings
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isPrimary = true;

    //matrices
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

    //set new camera as primary
    void SetAsPrimary();
};