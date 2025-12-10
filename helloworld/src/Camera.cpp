#include "Application.h"
#include "Camera.h"
#include "Input.h"
#include "Window.h"
#include "GUIManager.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "SceneManager.h"
#include <glm/gtx/string_cast.hpp>
#include <imgui.h> 
#include <cfloat>
#include <functional>
#include "RenderMeshComponent.h"

//helper function
void NormalizePlane(Plane& plane) {
	float length = glm::length(plane.normal);
	//safety check to prevent division by zero
	if (length > 1e-6f) {
		plane.normal /= length;
		plane.distance /= length;
	}
}

namespace {
	// Ray-AABB intersection (slab method). returns true and tOut is the distance to the intersection point.
	bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& tOut) {
		float tmin = (aabbMin.x - rayOrigin.x) / (rayDir.x == 0.0f ? 1e-8f : rayDir.x);
		float tmax = (aabbMax.x - rayOrigin.x) / (rayDir.x == 0.0f ? 1e-8f : rayDir.x);
		if (tmin > tmax) std::swap(tmin, tmax);

		float tymin = (aabbMin.y - rayOrigin.y) / (rayDir.y == 0.0f ? 1e-8f : rayDir.y);
		float tymax = (aabbMax.y - rayOrigin.y) / (rayDir.y == 0.0f ? 1e-8f : rayDir.y);
		if (tymin > tymax) std::swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax)) return false;

		if (tymin > tmin) tmin = tymin;
		if (tymax < tmax) tmax = tymax;

		float tzmin = (aabbMin.z - rayOrigin.z) / (rayDir.z == 0.0f ? 1e-8f : rayDir.z);
		float tzmax = (aabbMax.z - rayOrigin.z) / (rayDir.z == 0.0f ? 1e-8f : rayDir.z);
		if (tzmin > tzmax) std::swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax)) return false;

		if (tzmin > tmin) tmin = tzmin;
		if (tzmax < tmax) tmax = tzmax;

		//if tmax is negative, the AABB is behind the ray
		if (tmax < 0.0f) return false;

		// tmin is the distance to the intersection point
		tOut = (tmin >= 0.0f) ? tmin : tmax;
		return true;
	}

	//world AABB of a GameObject
	void ComputeWorldAABB(const std::shared_ptr<GameObject>& go, glm::vec3& outMin, glm::vec3& outMax) {
		outMin = glm::vec3(std::numeric_limits<float>::max());
		outMax = glm::vec3(std::numeric_limits<float>::lowest());

		if (!go) return;
		auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));
		auto transformComp = std::dynamic_pointer_cast<TransformComponent>(go->GetComponent(ComponentType::TRANSFORM));
		if (!renderComp || !transformComp) return;
		auto mesh = renderComp->GetMesh();
		if (!mesh) return;

		glm::mat4 worldMatrix = transformComp->GetGlobalTransform();
		glm::vec3 localMin = mesh->minAABB;
		glm::vec3 localMax = mesh->maxAABB;

		std::vector<glm::vec3> localCorners = {
			glm::vec3(localMin.x, localMin.y, localMin.z),
			glm::vec3(localMax.x, localMin.y, localMin.z),
			glm::vec3(localMin.x, localMax.y, localMin.z),
			glm::vec3(localMin.x, localMin.y, localMax.z),
			glm::vec3(localMax.x, localMax.y, localMin.z),
			glm::vec3(localMax.x, localMin.y, localMax.z),
			glm::vec3(localMin.x, localMax.y, localMax.z),
			glm::vec3(localMax.x, localMax.y, localMax.z)
		};

		for (const auto& corner : localCorners) {
			glm::vec4 wc = worldMatrix * glm::vec4(corner, 1.0f);
			glm::vec3 p = glm::vec3(wc);
			outMin = glm::min(outMin, p);
			outMax = glm::max(outMax, p);
		}
	}
}

Camera::Camera() : Module()
{
	name = "camera";
	// Initialize defaults here
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
	yaw = -90.0f; // Mirando hacia -Z (lo estándar en OpenGL)
	pitch = 0.0f;
	fov = 45.0f;
	distance = glm::length(cameraPos - targetPos); // Distancia inicial (3.0f)

	firstMouse = true;
	lastX = 0.0f;
	lastY = 0.0f;
	xpos = 0.0f;
	ypos = 0.0f;
}

Camera::~Camera()
{
}

bool Camera::Start()
{
	// Inicialización de matrices
	int windowW, windowH;
	Application::GetInstance().window->GetSize(windowW, windowH);

	UpdateCameraVectors();

	return true;
}

bool Camera::Update(float dt)
{
	//camera controls
	float cameraSpeed;


	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
		cameraSpeed = 0.20f;
	else
		cameraSpeed = 0.05f;

	xpos = Application::GetInstance().input.get()->GetMousePosition().x;
	ypos = Application::GetInstance().input.get()->GetMousePosition().y;
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	//Right Click
	if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT &&
		Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) != KEY_REPEAT)
	{
		ProcessKeyboardMovement(cameraSpeed);
		ProcessMouseRotation(xoffset, yoffset, 0.1f);
		UpdateCameraVectors();

		targetPos = cameraPos + cameraFront * distance;
	}

	if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP &&
		Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP &&
		Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_UP)
		firstMouse = true;



	//Alt + mouse
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) // Orbitar
		{
			ProcessMouseRotation(xoffset, yoffset, 0.3f);
		}
		else if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_REPEAT) // Pan
		{
			ProcessPan(xoffset, -yoffset);
		}
		else if (Application::GetInstance().input.get()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT) // Dolly
		{
			float combinedDelta = xoffset - yoffset;
			ProcessScrollZoom(combinedDelta, false);
			cameraPos = targetPos - cameraFront * distance;
		}
	}

	lastX = xpos;
	lastY = ypos;

	// Mouse Wheel
	float wheelDelta = Application::GetInstance().input.get()->GetMouseWheelDeltaY();
	if (std::abs(wheelDelta) > 10000.0f)
		wheelDelta = 0.0f;

	if (wheelDelta != 0.0f)
	{
		ProcessScrollZoom(wheelDelta, true);
		if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
		{
			cameraPos = targetPos - cameraFront * distance;
		}
	}
	Application::GetInstance().input.get()->SetMouseWheelDeltaY(0);

	FocusObject(false);
	UpdateCameraVectors();

	int windowW, windowH;
	Application::GetInstance().window->GetSize(windowW, windowH);
	RecalculateMatrices(windowW, windowH);
	ExtractFrustumPlanes();

	//click selection
	Input* input = Application::GetInstance().input.get();
	if (input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN) {
		//avoids selecting when clicking on GUI
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse && !Application::GetInstance().guiManager->assetsViewerIsHovered) {
			auto scene = Application::GetInstance().sceneManager->GetActiveScene();
			if (scene) {
				int mx = input->mouseX;
				int my = input->mouseY;
				glm::vec3 rayDir = input->MouseRay(mx, my, projectionMat, viewMat);
				glm::vec3 rayOrigin = cameraPos;

				std::shared_ptr<GameObject> bestHit = nullptr;
				float bestT = FLT_MAX;

				// iterates over all GameObjects in the scene
				std::function<void(const std::shared_ptr<GameObject>&)> Traverse;
				Traverse = [&](const std::shared_ptr<GameObject>& go) {
					if (!go) return;

					// ignore empty root objects
					if (!go->IsActive()) {
						for (auto& child : go->GetChildren()) Traverse(child);
						return;
					}

					// if it has a mesh renderer and transform, test intersection
	
					auto renderComp = std::dynamic_pointer_cast<RenderMeshComponent>(go->GetComponent(ComponentType::MESH_RENDERER));
					auto transformComp = std::dynamic_pointer_cast<TransformComponent>(go->GetComponent(ComponentType::TRANSFORM));
					if (renderComp && transformComp) {
						glm::vec3 worldMin, worldMax;
						ComputeWorldAABB(go, worldMin, worldMax);

						// if aabb is valid, test intersection
						if (worldMin.x <= worldMax.x && worldMin.y <= worldMax.y && worldMin.z <= worldMax.z) {
							float t = 0.0f;
							if (RayIntersectsAABB(rayOrigin, rayDir, worldMin, worldMax, t)) {
								if (t < bestT) {
									bestT = t;
									bestHit = go;
								}
							}
						}
					}

					// iterate children
					for (auto& child : go->GetChildren()) {
						Traverse(child);
					}
				};

				// from root, traverse all
				auto root = scene->GetRoot();
				if (root) {
					Traverse(root);
				}

				// set guimanager selection
				auto gui = Application::GetInstance().guiManager.get();

				if (gui->selectedObject) {
					gui->selectedObject->isSelected = false;
				}
				if (bestHit) {
					gui->selectedObject = bestHit;
					bestHit->isSelected = true;
					LOG("Selected GameObject: %s (UUID: %llu)", bestHit->GetName().c_str(), bestHit->GetUUID());
				}
				else {
					gui->selectedObject = nullptr;
					LOG("No GameObject selected by click");
				}
			}
		}
	}

	return true;
}


void Camera::ProcessMouseRotation(float xoffset, float yoffset, float sensitivity)
{
	if (firstMouse)
	{
		lastX = Application::GetInstance().input.get()->GetMousePosition().x;
		lastY = Application::GetInstance().input.get()->GetMousePosition().y;
		firstMouse = false;
		return;
	}

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;


	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;


	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		UpdateCameraVectors();
		cameraPos = targetPos - cameraFront * distance;
	}
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void Camera::ProcessKeyboardMovement(float actualSpeed)
{
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		cameraPos += actualSpeed * cameraFront;
	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		cameraPos -= actualSpeed * cameraFront;

	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		cameraPos += cameraRight * actualSpeed;
	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		cameraPos -= cameraRight * actualSpeed;

	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
		cameraPos += worldUp * actualSpeed;
	if (Application::GetInstance().input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
		cameraPos -= worldUp * actualSpeed;
}

void Camera::ProcessPan(float xoffset, float yoffset)
{
	float panSpeed = 0.01f * (distance / 2);

	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 cameraUpVector = glm::normalize(glm::cross(cameraRight, cameraFront));

	targetPos -= cameraRight * xoffset * panSpeed;
	targetPos += cameraUpVector * yoffset * panSpeed;
	cameraPos = targetPos - cameraFront * distance;
}

void Camera::ProcessScrollZoom(float delta, bool isMouseScroll)
{
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT || !isMouseScroll) {

		float dollyMultiplier = isMouseScroll ? 0.5f : (0.01f * distance);
		distance -= delta * dollyMultiplier;

		if (distance < 0.1f)
			distance = 0.1f;
	}
	else {
		float zoomSpeed = 5.0f;
		fov -= delta * zoomSpeed;
		if (fov < 1.0f) fov = 1.0f;
		if (fov > 90.0f) fov = 90.0f;
	}
}

void Camera::FocusObject(bool firstTime) {
	if (Application::GetInstance().input.get()->GetKey(SDL_SCANCODE_F) == KEY_DOWN || firstTime)
	{
		std::shared_ptr<GameObject> selectedObj = nullptr;

		if (firstTime) {
			// Find the first valid object with a mesh (not the root)
			auto scene = Application::GetInstance().sceneManager->GetActiveScene();
			if (scene) {
				const auto& allObjects = scene->GetAllGameObjects();

				// Skip root (index 0), find first object with a mesh
				for (size_t i = 1; i < allObjects.size(); i++) {
					auto obj = allObjects[i];
					if (obj && obj->GetComponent(ComponentType::MESH_RENDERER)) {
						selectedObj = obj;
						break;
					}
				}
			}
		}
		else {
			// Use the user's selected object
			selectedObj = Application::GetInstance().guiManager->selectedObject;
		}

		// Only focus if we found a valid object
		if (selectedObj)
		{
			auto transformComp = std::dynamic_pointer_cast<TransformComponent>(
				selectedObj->GetComponent(ComponentType::TRANSFORM)
			);

			if (transformComp)
			{
				glm::vec3 targetPosition = transformComp->GetWorldPosition();
				UpdateCameraVectors();
				const float focusDistance = 7.0f;
				const float heightOffset = 1.0f;
				cameraPos = targetPosition - cameraFront * focusDistance + glm::vec3(0, heightOffset, 0);
				targetPos = targetPosition;
				distance = glm::length(cameraPos - targetPos);
				glm::vec3 direction = glm::normalize(targetPos - cameraPos);
				yaw = glm::degrees(atan2(direction.z, direction.x));
				pitch = glm::degrees(asin(direction.y));

				LOG("Camera focused on '%s'", selectedObj->GetName().c_str());
			}
		}
		else {
			LOG("No valid object to focus on");
		}
	}
}

void Camera::RecalculateMatrices(int windowW, int windowH)
{
	float aspectRatio = (float)Application::GetInstance().window.get()->width / (float)Application::GetInstance().window.get()->height;
	projectionMat = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	viewMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::ExtractFrustumPlanes() {
	glm::mat4 clip = projectionMat * viewMat; //combined natrix

	//planes are extracted using the columns of the clip matrix

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