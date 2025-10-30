#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "GameObject.h"
#include "Shader.h"


RenderMeshComponent::RenderMeshComponent(std::shared_ptr<GameObject> owner)
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

void RenderMeshComponent::SetMesh(std::shared_ptr<Mesh> newMesh) {
    mesh = newMesh;
}

void RenderMeshComponent::Render(Shader* shader) {
    if (!mesh || !active || !shader) return;
    
    auto sharedOwner = owner.lock();
    if (!sharedOwner) return;
    // Get transform component to apply transformations
    
    auto transform = std::dynamic_pointer_cast<TransformComponent>(sharedOwner->GetComponent(ComponentType::TRANSFORM));
    if (!transform)
        return;
    
    
   
    auto material = std::dynamic_pointer_cast<MaterialComponent>(sharedOwner->GetComponent(ComponentType::MATERIAL));
    if (!material)
        return;
    
    
    // Apply transformation matrix
    
    glm::mat4 modelMatrix = transform->GetGlobalTransform();
    
    

    // Set the model matrix in the shader
    shader->setMat4("model", modelMatrix);

    mesh->Draw(*shader);
}


