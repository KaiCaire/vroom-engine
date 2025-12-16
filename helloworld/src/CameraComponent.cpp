#include "CameraComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Log.h"
#include "glm/gtx/quaternion.hpp"

// Helper function from Camera.cpp
void CameraComponent::NormalizePlane(Plane& plane) {
	float length = glm::length(plane.normal);
	if (length > 1e-6f) {
		plane.normal /= length;
		plane.distance /= length;
	}
}

CameraComponent::CameraComponent(std::shared_ptr<GameObject> owner)
    : Component(owner, ComponentType::CAMERA) {
}

void CameraComponent::Enable() {}
void CameraComponent::Update() {}
void CameraComponent::Disable() {}
void CameraComponent::OnEditor() { /* TODO: ImGui implementation */ }

glm::mat4 CameraComponent::GetViewMatrix() const {
    auto ownerGO = GetOwner();
    if (!ownerGO) return glm::mat4(1.0f);

    auto transform = std::dynamic_pointer_cast<TransformComponent>(ownerGO->GetComponent(ComponentType::TRANSFORM));
    if (!transform) return glm::mat4(1.0f);

    glm::quat worldRot = transform->GetWorldRotation();
    glm::vec3 worldPos = transform->GetWorldPosition();

    glm::vec3 forward = glm::normalize(worldRot * glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 up = glm::normalize(worldRot * glm::vec3(0.0f, 1.0f, 0.0f));

    viewMatrix = glm::lookAt(worldPos, worldPos + forward, up);
    return viewMatrix;
}

glm::mat4 CameraComponent::GetProjectionMatrix() const {
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    return projectionMatrix;
}

void CameraComponent::ExtractFrustumPlanes(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
    glm::mat4 clip = projectionMatrix * viewMatrix;

	//right plane
	frustum.planes[PLANE_RIGHT].normal.x = clip[0][3] - clip[0][0];
	frustum.planes[PLANE_RIGHT].normal.y = clip[1][3] - clip[1][0];
	frustum.planes[PLANE_RIGHT].normal.z = clip[2][3] - clip[2][0];
	frustum.planes[PLANE_RIGHT].distance = clip[3][3] - clip[3][0];
	NormalizePlane(frustum.planes[PLANE_RIGHT]);

	//left plane
	frustum.planes[PLANE_LEFT].normal.x = clip[0][3] + clip[0][0];
	frustum.planes[PLANE_LEFT].normal.y = clip[1][3] + clip[1][0];
	frustum.planes[PLANE_LEFT].normal.z = clip[2][3] + clip[2][0];
	frustum.planes[PLANE_LEFT].distance = clip[3][3] + clip[3][0];
	NormalizePlane(frustum.planes[PLANE_LEFT]);

	//bottom plane
	frustum.planes[PLANE_BOTTOM].normal.x = clip[0][3] + clip[0][1];
	frustum.planes[PLANE_BOTTOM].normal.y = clip[1][3] + clip[1][1];
	frustum.planes[PLANE_BOTTOM].normal.z = clip[2][3] + clip[2][1];
	frustum.planes[PLANE_BOTTOM].distance = clip[3][3] + clip[3][1];
	NormalizePlane(frustum.planes[PLANE_BOTTOM]);

	//top plane
	frustum.planes[PLANE_TOP].normal.x = clip[0][3] - clip[0][1];
	frustum.planes[PLANE_TOP].normal.y = clip[1][3] - clip[1][1];
	frustum.planes[PLANE_TOP].normal.z = clip[2][3] - clip[2][1];
	frustum.planes[PLANE_TOP].distance = clip[3][3] - clip[3][1];
	NormalizePlane(frustum.planes[PLANE_TOP]);

	//far plane
	frustum.planes[PLANE_FAR].normal.x = clip[0][3] - clip[0][2];
	frustum.planes[PLANE_FAR].normal.y = clip[1][3] - clip[1][2];
	frustum.planes[PLANE_FAR].normal.z = clip[2][3] - clip[2][2];
	frustum.planes[PLANE_FAR].distance = clip[3][3] - clip[3][2];
	NormalizePlane(frustum.planes[PLANE_FAR]);

	//near plane
	frustum.planes[PLANE_NEAR].normal.x = clip[0][3] + clip[0][2];
	frustum.planes[PLANE_NEAR].normal.y = clip[1][3] + clip[1][2];
	frustum.planes[PLANE_NEAR].normal.z = clip[2][3] + clip[2][2];
	frustum.planes[PLANE_NEAR].distance = clip[3][3] + clip[3][2];
	NormalizePlane(frustum.planes[PLANE_NEAR]);
}