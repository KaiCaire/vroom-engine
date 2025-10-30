#include "Model.h"
#include "Application.h"
#include "OpenGL.h"
#include "FileSystem.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
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

    rootGameObject = new GameObject("RootNode");
    processNodeWithGameObjects(scene->mRootNode, scene, rootGameObject);

    LOG("Finished Loading Model");

    LOG("=== MODEL LOADING SUMMARY ===");
    LOG("Total GameObjects created: %d", gameObjects.size());
    LOG("Total Meshes processed: %d", meshes.size());
    LOG("Root GameObject: '%s'", rootGameObject ? rootGameObject->GetName().c_str() : "NULL");

    // Mostrar la jerarquía completa
    if (rootGameObject) {
        LOG("=== HIERARCHY ===");
        LogGameObjectHierarchy(rootGameObject, 0);
    }
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

Model::~Model() {
    // Cleanup gameobjects
    for (GameObject* go : gameObjects) {
        delete go;
    }
    gameObjects.clear();
}

void Model::processNodeWithGameObjects(aiNode* node, const aiScene* scene, GameObject* parent)
{
    // Create GameObject for this node
    GameObject* gameObject = new GameObject(node->mName.C_Str());
    gameObjects.push_back(gameObject);

    // AÑADIR AQUÍ:
    LOG("Created GameObject: '%s' (Parent: '%s')",
        gameObject->GetName().c_str(),
        parent ? parent->GetName().c_str() : "NULL");

    // Create Transform component and load transformation
    Component* transformComp = gameObject->AddComponent(ComponentType::TRANSFORM);
    TransformComponent* transform = static_cast<TransformComponent*>(transformComp);

    // AÑADIR AQUÍ:
    LOG("  - Added Transform component to '%s'", gameObject->GetName().c_str());

    // Decompose Assimp transformation matrix
    aiVector3D position, scaling;
    aiQuaternion rotation;
    node->mTransformation.Decompose(scaling, rotation, position);

    transform->SetPosition(glm::vec3(position.x, position.y, position.z));
    transform->SetRotation(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
    transform->SetScale(glm::vec3(scaling.x, scaling.y, scaling.z));

    // AÑADIR AQUÍ:
    LOG("  - Transform: Pos(%.2f, %.2f, %.2f) Scale(%.2f, %.2f, %.2f)",
        position.x, position.y, position.z,
        scaling.x, scaling.y, scaling.z);

    // Set parent
    if (parent) {
        gameObject->SetParent(parent);
        // AÑADIR AQUÍ:
        LOG("  - Set parent to '%s'", parent->GetName().c_str());
    }

    // Process all meshes for this node
    // AÑADIR AQUÍ:
    LOG("  - Processing %d meshes for '%s'", node->mNumMeshes, gameObject->GetName().c_str());

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // ... resto del código
    }

    // Process children recursively
    // AÑADIR AQUÍ:
    LOG("  - Processing %d children for '%s'", node->mNumChildren, gameObject->GetName().c_str());

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNodeWithGameObjects(node->mChildren[i], scene, gameObject);
    }
}

void Model::createComponentsForMesh(GameObject* gameObject, aiMesh* aiMesh, const aiScene* scene)
{
    // AÑADIR AL INICIO:
    LOG("Creating components for mesh in GameObject '%s'", gameObject->GetName().c_str());
    LOG("  - Vertices: %d, Faces: %d", aiMesh->mNumVertices, aiMesh->mNumFaces);

    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // ... proceso de vertices e indices ...

    // Load textures
    if (aiMesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // AÑADIR AQUÍ:
        LOG("  - Loaded %d diffuse textures", diffuseMaps.size());

        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // AÑADIR AQUÍ:
        LOG("  - Loaded %d specular textures", specularMaps.size());
    }

    // Create the actual Mesh object and store it
    Mesh* mesh = new Mesh(vertices, indices, textures);
    meshes.push_back(*mesh);

    // Create MeshRenderer component
    Component* rendererComp = gameObject->AddComponent(ComponentType::MESH_RENDERER);
    RenderMeshComponent* renderer = static_cast<RenderMeshComponent*>(rendererComp);
    renderer->SetMesh(mesh);

    // AÑADIR AQUÍ:
    LOG("  - Added MeshRenderer component to '%s'", gameObject->GetName().c_str());

    // Create Material component
    Component* materialComp = gameObject->AddComponent(ComponentType::MATERIAL);
    MaterialComponent* matComponent = static_cast<MaterialComponent*>(materialComp);

    // AÑADIR AQUÍ:
    LOG("  - Added Material component to '%s'", gameObject->GetName().c_str());

    // Load material properties from Assimp
    if (aiMesh->mMaterialIndex >= 0)
    {
        aiMaterial* aiMat = scene->mMaterials[aiMesh->mMaterialIndex];

        // Get color
        aiColor4D color;
        if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
            matComponent->SetColor(glm::vec4(color.r, color.g, color.b, color.a));
            // AÑADIR AQUÍ:
            LOG("    - Material color: (%.2f, %.2f, %.2f, %.2f)",
                color.r, color.g, color.b, color.a);
        }

        // Get shininess
        float shininess;
        if (aiGetMaterialFloat(aiMat, AI_MATKEY_SHININESS, &shininess) == AI_SUCCESS) {
            matComponent->SetShininess(shininess);
            // AÑADIR AQUÍ:
            LOG("    - Material shininess: %.2f", shininess);
        }
    }
}

void Model::LogGameObjectHierarchy(GameObject* go, int depth) {
    if (!go) return;

    string indent(depth * 2, ' ');
    LOG("%s- '%s' (Active: %s, Components: %d, Children: %d)",
        indent.c_str(),
        go->GetName().c_str(),
        go->IsActive() ? "Yes" : "No",
        // Aquí necesitarías un getter para contar componentes
        0, // Por ahora 0, o añade un método GetComponentCount() en GameObject
        go->GetChildren().size());

    for (GameObject* child : go->GetChildren()) {
        LogGameObjectHierarchy(child, depth + 1);
    }
}
