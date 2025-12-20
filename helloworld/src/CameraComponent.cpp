#include "CameraComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include <glm/gtc/matrix_transform.hpp>

CameraComponent::CameraComponent(std::shared_ptr<GameObject> owner)
    : Component(owner, ComponentType::CAMERA) {
}

glm::mat4 CameraComponent::GetViewMatrix() const {
    auto transform = std::dynamic_pointer_cast<TransformComponent>(GetOwner()->GetComponent(ComponentType::TRANSFORM));
    glm::vec3 pos = transform->GetWorldPosition();
    glm::vec3 front = transform->GetForward(); 
    glm::vec3 up = glm::vec3(0, 1, 0);
    return glm::lookAt(pos, pos + front, up);
}

glm::mat4 CameraComponent::GetProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void CameraComponent::Update()
{
}