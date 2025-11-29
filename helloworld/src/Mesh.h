

#pragma once
#include "Resource.h"
#include <vector>
#include <memory>  
#include <glm/glm.hpp>
#include "Shader.h"
#include "UUID.h"



struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 texCoord;
};

//struct Texture {
//    unsigned int id;
//    std::string mapType;
//    std::string path;
//    // Add TextureFromFile method here or in a cpp
//   /* void TextureFromFile(const std::string& path, const char* fileName);*/
//};

class Texture;

class Mesh : public Resource {
public:
    // Mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<glm::vec3> normals;
    
    // OpenGL buffers
    unsigned int VAO, VBO, EBO;

    // Constructors
    Mesh();
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures);

    // Draw the mesh
    void Draw(Shader& shader);

    

    // Set mesh data (useful for updating mesh after creation)
    void SetMeshData(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds, const std::vector<std::shared_ptr<Texture>>& texs);



    // Resource interface implementation
    bool LoadToMemory() override;
    void UnloadFromMemory() override;

    void CalculateNormals();
    bool drawVertNormals = false;
    bool drawFaceNormals = false;

private:
    void setupMesh();
};