#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <string>

class MaterialComponent : public Component {
public:
    MaterialComponent(std::shared_ptr<GameObject> owner);
    ~MaterialComponent() override;

    // Component interface
    void Enable() override;
    void Update() override;
    void Disable() override;
    void OnEditor() override;

    // Material properties
    void SetColor(const glm::vec4& col) { color = col; }
    glm::vec4 GetColor() const { return color; }

    void SetShininess(float shine) { shininess = shine; }
    float GetShininess() const { return shininess; }

    void SetMetallic(float metal) { metallic = metal; }
    float GetMetallic() const { return metallic; }

    void SetRoughness(float rough) { roughness = rough; }
    float GetRoughness() const { return roughness; }

private:
    // Material properties
    glm::vec4 color;
    float shininess;
    float metallic;
    float roughness;
};