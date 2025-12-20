#include "MaterialComponent.h"
#include "GameObject.h"
#include "Application.h"

MaterialComponent::MaterialComponent(std::shared_ptr<GameObject> owner)
    : Component(owner, ComponentType::MATERIAL),
    diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    shininess(32.0f),
    metallic(0.0f),
    roughness(0.5f) {

    
    
}

MaterialComponent::~MaterialComponent() {
    if (diffuseMap) diffuseMap->RemoveReference();
    if (specularMap) specularMap->RemoveReference();
    if (normalMap) normalMap->RemoveReference();
    if (metallicMap) metallicMap->RemoveReference();
    if (roughnessMap) roughnessMap->RemoveReference();
    if (aoMap) aoMap->RemoveReference();
}

void MaterialComponent::Enable() {
    // Material enabled
}

void MaterialComponent::Update() {
    // Materials don't need per-frame updates
}

void MaterialComponent::Disable() {
    // Material disabled
}

void MaterialComponent::OnEditor() {
    //laracode aqui imgui
}


