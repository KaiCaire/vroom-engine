#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "GameObject.h"
#include "Shader.h"


RenderMeshComponent::RenderMeshComponent(GameObject* owner)
    : Component(owner, ComponentType::MESH_RENDERER),
    mesh(nullptr),
    castShadows(true),
    receiveShadows(true) {
}

RenderMeshComponent::~RenderMeshComponent() {
    
    mesh = nullptr;
}

void RenderMeshComponent::Enable() {
    // Component enabled
}

void RenderMeshComponent::Update() {
    
}

void RenderMeshComponent::Disable() {
    // Component disabled
}

void RenderMeshComponent::OnEditor() {
    //laracode aqui
}

void RenderMeshComponent::SetMesh(Mesh* newMesh) {
    mesh = newMesh;
}

void RenderMeshComponent::Render(Shader* shader) {
    if (!mesh || !active || !shader) return;

    // Get transform component to apply transformations
    TransformComponent* transform =
        static_cast<TransformComponent*>(owner->GetComponent(ComponentType::TRANSFORM));
    if (!transform)
        return;

    MaterialComponent* material =
        static_cast<MaterialComponent*>(owner->GetComponent(ComponentType::MATERIAL));

    // Apply transformation matrix
    glm::mat4 modelMatrix = transform->GetGlobalMatrix();

    // Set the model matrix in the shader
    shader->setMat4("model", modelMatrix);

    mesh->Draw(*shader);
}