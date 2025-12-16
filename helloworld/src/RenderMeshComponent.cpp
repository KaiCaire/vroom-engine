#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "GameObject.h"
#include "Shader.h"


RenderMeshComponent::RenderMeshComponent(std::shared_ptr<GameObject> owner) : Component(owner, ComponentType::MESH_RENDERER), mesh(nullptr) {}

RenderMeshComponent::~RenderMeshComponent() {

    if (mesh) {
        mesh->RemoveReference();
    }
    
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

void RenderMeshComponent::SetMesh(std::shared_ptr<ResourceMesh> newMesh) {
    if (mesh && meshUUID != 0) {
        Application::GetInstance().resourceManager->RemoveReference(meshUUID);
    }

    mesh = newMesh;
    if (newMesh) {
        meshUUID = newMesh->GetUUID();

        if (meshUUID != 0) {
            newMesh->AddReference();
            LOG("RenderMeshComponent assigned Mesh UUID: %llu and added reference.", meshUUID);
        }
        
    }
    else meshUUID = 0;
}

void RenderMeshComponent::Render(Shader* shader) {
    if (!mesh || !active || !shader) return;
    
    auto owner = GetOwner();
    if (!owner) return;
    // Get transform component to apply transformations
    
    auto transform = std::dynamic_pointer_cast<TransformComponent>(owner->GetComponent(ComponentType::TRANSFORM));
    if (!transform)
        return;

    auto material = std::dynamic_pointer_cast<MaterialComponent>(owner->GetComponent(ComponentType::MATERIAL));
    if (!material)
        return;
    
    
    // Apply transformation matrix
    
    glm::mat4 modelMatrix = transform->GetGlobalTransform();
    
    

    // Set the model matrix in the shader
    shader->setMat4("model", modelMatrix);

    mesh->Draw(*shader, material.get());
}

