#pragma once
#include "Resource.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Textures.h"
#include <vector>
#include <SDL3/SDL_opengl.h>
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"


using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 texCoord;
};

class Texture;
 

class Mesh {
public:
    // mesh data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;

    vector<glm::vec3>    normals;
    

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    ~Mesh();
    void CalculateNormals();
    void Draw(Shader &shader);
    bool drawVertNormals = false;
    bool drawFaceNormals = false;

    void SetUUID(VroomUUID _uuid) 
    { 
        uuid = _uuid; 
    }

    //void SaveBin() override;
    //void LoadBin() override;

    //void SaveMeta() const override;
    //void LoadMeta() override;
    //

private:
    //  render data
    unsigned int VAO, VBO, EBO;
    VroomUUID uuid = 0;
    void setupMesh();
    
};