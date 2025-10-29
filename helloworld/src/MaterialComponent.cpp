#include "MaterialComponent.h"
#include "GameObject.h"

MaterialComponent::MaterialComponent(GameObject* owner)
    : Component(owner, ComponentType::MATERIAL),
    color(1.0f, 1.0f, 1.0f, 1.0f),
    shininess(32.0f),
    metallic(0.0f),
    roughness(0.5f) {
}

MaterialComponent::~MaterialComponent() {
    // Nothing to clean up
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