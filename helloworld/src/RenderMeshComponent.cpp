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

    auto transform = std::dynamic_pointer_cast<TransformComponent>(owner->GetComponent(ComponentType::TRANSFORM));
    if (!transform) return;

    // 1. Get the material but DO NOT 'return' if it's null
    auto material = std::dynamic_pointer_cast<MaterialComponent>(owner->GetComponent(ComponentType::MATERIAL));

    // 2. Apply transformations
    glm::mat4 modelMatrix = transform->GetGlobalTransform();
    shader->setMat4("model", modelMatrix);

    // 3. Pass the material (even if null) to the mesh
    // ResourceMesh::Draw will use its own fallback logic if material is nullptr!
    mesh->Draw(*shader, material.get());
}

