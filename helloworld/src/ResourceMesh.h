#pragma once
#include "Resource.h"
#include <vector>
#include <memory>  
#include <glm/glm.hpp>
#include "Shader.h"
#include "UUID.h"
#include "MaterialComponent.h"


struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 texCoord;
};

class ResourceTexture;

class ResourceMesh : public Resource {
public:
    // Mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<ResourceTexture>> textures;
    std::vector<glm::vec3> normals;

    uint numVertices = 0;
    uint numIndices = 0;

    //bounding boxes
    glm::vec3 minAABB = glm::vec3(0.0f); 
    glm::vec3 maxAABB = glm::vec3(0.0f);
    
    // OpenGL buffers
    unsigned int VAO, VBO, EBO;

    bool isLoadedToGPU;

    // Constructors
    ResourceMesh();
    ResourceMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, 
        std::vector<std::shared_ptr<ResourceTexture>> textures = {}); //textures are optional
    ResourceMesh(std::shared_ptr<ResourceMesh> mesh);
    // Draw the mesh

    ~ResourceMesh() override;
    void Draw(Shader& shader, MaterialComponent* material);
    /*void Draw(Shader& shader);*/

    

    // Set mesh data (useful for updating mesh after creation)
    void SetMeshData(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds/*, const std::vector<std::shared_ptr<ResourceTexture>>& texs*/);
    void LoadToGPU();
    void UnloadFromGPU();


    // Resource interface implementation
    //bool LoadToMemory() override;
    //void UnloadFromMemory() override;
    void SaveBin() override;
    void LoadBin() override;
    void SaveMeta() const override;
    void LoadMeta() override;
    void FreeMemory() override;

    void CalculateNormals();
    bool drawVertNormals = false;
    bool drawFaceNormals = false;

private:
    
};