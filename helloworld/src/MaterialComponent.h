#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <string>
#include "Textures.h"
#include <memory>

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
    void SetDiffuseColor(const glm::vec4& col) { diffuseColor = col; }
    glm::vec4 GetDiffuseColor() const { return diffuseColor; }

    void SetShininess(float shine) { shininess = shine; }
    float GetShininess() const { return shininess; }

    void SetMetallic(float metal) { metallic = metal; }
    float GetMetallic() const { return metallic; }

    void SetRoughness(float rough) { roughness = rough; }
    float GetRoughness() const { return roughness; }

    void SetDiffuseMap(std::shared_ptr<Texture> tex) { diffuseMap = tex; }
    std::shared_ptr<Texture> GetDiffuseMap() const { return diffuseMap; }

    void SetNormalMap(std::shared_ptr<Texture> tex) { normalMap = tex; }
    std::shared_ptr<Texture> GetNormalMap() const { return normalMap; }

    void SetSpecularMap(std::shared_ptr<Texture> tex) { specularMap = tex; }
    std::shared_ptr<Texture> GetSpecularMap() const { return specularMap; }

    void SetMetallicMap(std::shared_ptr<Texture> tex) { metallicMap = tex; }
    std::shared_ptr<Texture> GetMetallicMap() const { return metallicMap; }

    void SetRoughnessMap(std::shared_ptr<Texture> tex) { roughnessMap = tex; }
    std::shared_ptr<Texture> GetRoughnessMap() const { return roughnessMap; }

    void SetAOMap(std::shared_ptr<Texture> tex) { aoMap = tex; }
    std::shared_ptr<Texture> GetAOMap() const { return aoMap; }

private:
    // Material properties
    glm::vec4 diffuseColor = glm::vec4(1.0f);
    glm::vec3 specularColor = glm::vec3(1.0f);
    glm::vec3 emissiveColor = glm::vec3(0.0f);

    // Material properties
    float shininess = 32.0f;
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    
    /*Texture defaultColorTex;*/
    //can be null
    std::shared_ptr<Texture> diffuseMap;
    std::shared_ptr<Texture> specularMap;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Texture> metallicMap;
    std::shared_ptr<Texture> roughnessMap;
    std::shared_ptr<Texture> aoMap;

   

};