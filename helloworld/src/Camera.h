#pragma once
#include "Module.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class Camera : public Module 
{
public:

    Camera();
    virtual ~Camera();

    bool Start() override;
    bool Update(float dt) override;

    // Core camera state
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 targetPos;

    // Transform data
    float yaw;
    float pitch;
    float fov;
    float distance;

    // Mouse state (necesario para el modo de coordenadas absolutas)
    bool firstMouse;
    float lastX;
    float lastY;
    float xpos;
    float ypos;

    // Matrices (accesibles desde OpenGL para renderizar)
    glm::mat4 viewMat;
    glm::mat4 projectionMat;
    bool wasInOrbitalMode = false;
    const float focusDistance = 7.0f;
    const float heightOffset = 1.0f;
    bool isFirstOrbital = true;

    // Helper functions (implementadas en Camera.cpp)
    void ProcessMouseRotation(float xoffset, float yoffset, float sensitivity);
    void UpdateCameraVectors();
    void ProcessKeyboardMovement(float actualSpeed);
    void ProcessPan(float xoffset, float yoffset);
    void ProcessScrollZoom(float delta, bool isMouseScroll);
    void FocusObject(bool firstTime);
    glm::vec3 GetWorldPosition() const { return cameraPos; }

private:
    void RecalculateMatrices(int windowW, int windowH);
};