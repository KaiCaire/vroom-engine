#include "Model.h"
#include "Application.h"
#include "OpenGL.h"
#include "FileSystem.h"
#include <string>
#include <vector>
#include "Mesh.h"
#include "assimp/importer.hpp"
#include "stb_image.h"
#include "Textures.h"
#include "Log.h"

using namespace std;


vector<Texture> textures_loaded;

void Model::loadModel(string path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    LOG("Finished Loading Model");
}

void Model::Draw(Shader& shader) {
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader); //loops over each of the meshes to call their respective Draw function
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        
        meshes.push_back(processMesh(mesh, scene));
        
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        //recursive method! we need to iterate the entire hierarchy until the very last of its children
        processNode(node->mChildren[i], scene);
        
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene){
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    //TODO: for each mesh, create a GameObject
    

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates! Otherwise you'll have empty vertices!
        

        // Position
        vertex.Position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Normals
        if (mesh->HasNormals()) {
            vertex.Normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }

        // Texture Coordinates
        if (mesh->mTextureCoords[0]) { // Check if mesh has texture coords
            vertex.texCoord = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }
        else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    // process indices

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process material
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
        vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    } 

    processedMeshes++;
    LOG("Processed mesh %d", processedMeshes);
    return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        //implementing skipping already loaded textures cuz without this, it is slow af
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.TextureFromFile(directory, str.C_Str());
            texture.mapType = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}




