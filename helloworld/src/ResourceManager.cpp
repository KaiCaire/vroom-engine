
#include "ResourceManager.h"
#include "Application.h"
#include "FileSystem.h"
#include "Importer.h"
#include "TextureImporter.h"
#include "MeshImporter.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "Log.h"
#include "ModelImporter.h"



ResourceManager::ResourceManager() : Module() {
    LOG("ResourceManager Constructor");
}

ResourceManager::~ResourceManager() {
    
}

bool ResourceManager::Start() {
    LOG("ResourceManager Start");

    // Create Library directories if they don't exist
    fs = Application::GetInstance().fileSystem.get();
    fs->CreateDir("Library");
    fs->CreateDir("Library/Meshes");
    fs->CreateDir("Library/Textures");
    fs->CreateDir("Library/Materials");

    //scan assets
    ScanAssetsFolder();

    return true;
}

bool ResourceManager::CleanUp() {
    LOG("ResourceManager Cleanup - Releasing %d resources", (int)resources.size());
    resources.clear();
    return true;
}

std::shared_ptr<Resource> ResourceManager::RequestResource(VroomUUID uuid) {
    // Check if already loaded
    auto it = resources.find(uuid);
    if (it != resources.end()) {
        LOG("Resource %llu already loaded (refCount: %d)", uuid, it->second->GetReferenceCount());
        it->second->AddReference();
        return it->second;
    }

    // Determine type from library file extension
    ResourceType type = ResourceType::UNKNOWN;

    std::string texturePath = "Library/Textures/" + std::to_string(uuid) + ".vroomtex";
    std::string meshPath = "Library/Meshes/" + std::to_string(uuid) + ".vroommesh";

    if (fs->Exists(texturePath.c_str())) {
        type = ResourceType::TEXTURE;
    }
    else if (fs->Exists(meshPath.c_str())) {
        type = ResourceType::MESH;
    }
    else {
        LOG("ERROR: Library file not found for UUID %llu", uuid);
        return nullptr;
    }

    // Create and load from library
    auto resource = CreateResource(type, uuid);
    if (resource && LoadResourceFromLibrary(resource)) {
        RegisterResource(resource);
        return resource;
    }

    return nullptr;
}

std::shared_ptr<Resource> ResourceManager::RequestResource(const std::string& assetsPath) {
   
    std::string normalizedPath = fs->NormalizePath(assetsPath.c_str());

    // Determine resource type from file extension
    std::string extension = fs->GetExtensionFromPath(assetsPath.c_str());
    ResourceType type = ResourceType::UNKNOWN;

    if (extension == "png" || extension == "jpg" || extension == "tga" || extension == "dds") {
        type = ResourceType::TEXTURE;
    }
    else if (extension == "fbx" || extension == "obj") {
        type = ResourceType::SCENE;
    }

    VroomUUID resUUID = 0;

    // Check if .meta exists
    std::string metaPath = normalizedPath + ".meta";
    if (fs->Exists(metaPath.c_str())) {
        LOG(".meta file found for %s, importing from library.", normalizedPath.c_str());
        /*return nullptr;*/

         // Load UUID from .meta
        resUUID = fs->GetUUIDFromMeta(metaPath.c_str());
        // Check if already loaded
        auto it = resources.find(resUUID);
        if (it != resources.end()) {
            LOG("Resource '%s' (UUID: %llu) already loaded in memory, increasing reference count.", normalizedPath.c_str(), resUUID);
            it->second->AddReference();

            if (LoadResourceToGPU(it->second))
                return it->second;
        }
        else {
            std::string libraryPath = "Library/Textures/" + std::to_string(resUUID) + ".vroomtex";

            if (!fs->Exists(libraryPath.c_str())) {
                LOG("Library file missing, reimporting from source");
                
            }
            else {
                auto res = CreateResource(type, resUUID);
                res->SetLibraryFilePath(libraryPath);

                if (LoadResourceFromLibrary(res)) {
                    RegisterResource(res);
                    LoadResourceToGPU(res);
                    return res;
                }
                else {
                    LOG("Failed to load from Library, reimporting");
                }
            }

        }

    }
    else {
        LOG("No .meta file found for %s, importing fresh", normalizedPath.c_str());

    }

    resUUID = ImportFile(normalizedPath, type);
    // Return the now-loaded resource
    return GetResourceByUUID(resUUID);
}

VroomUUID ResourceManager::ImportFile(const std::string& assetsPath, ResourceType type) {
    LOG("ResourceManager: Importing file '%s' (type: %d)", assetsPath.c_str(), (int)type);

    Importer* importer = Application::GetInstance().importer.get();
    VroomUUID uuid = 0;

    switch (type) {
    case ResourceType::TEXTURE:
    {
        auto texture = Application::GetInstance().importer.get()->textureImporter->Import(assetsPath);
        if (texture) {
            RegisterResource(texture);
            uuid = texture->GetUUID();
        }
        break;
    }
    case ResourceType::SCENE:
        /*auto mesh = Application::GetInstance().importer.get()->meshImporter->Import(assetsPath.c_str());*/
        Application::GetInstance().importer.get()->modelImporter->ImportScene(assetsPath.c_str());
        
        break;

    default:
        LOG("ERROR: Unknown resource type");
        break;
    }

    return uuid;
}

std::shared_ptr<Resource> ResourceManager::CreateResource(ResourceType type, VroomUUID uuid) {
    
    if (uuid == 0) {
        LOG("ERROR: Attempted to create resource with UUID 0!");
        return nullptr;  
    }
    
    std::shared_ptr<Resource> resource = nullptr;

    switch (type) {
    case ResourceType::MESH:
        resource = std::make_shared<ResourceMesh>();
        break;
    case ResourceType::TEXTURE:
        resource = std::make_shared<ResourceTexture>();
        break;
    default:
        LOG("ERROR: Unknown resource type");
        return nullptr;
    }

    if (resource) {
        resource->SetUUID(uuid);
        resources[uuid] = resource;
        LOG("Created resource (UUID: %llu, Type: %d)", uuid, (int)type);
    }

    return resource;
}

void ResourceManager::RegisterResource(std::shared_ptr<Resource> resource)
{
    if (!resource) {
        LOG("ERROR: Tried to register null resource");
        return;
    }

   
    VroomUUID uuid = resource->GetUUID();

    if (uuid == 0) {
        LOG("UUID not found, cannot register resource with UUID = 0");
        return;
    }

    // Check if already registered
    auto it = resources.find(uuid);
    if (it != resources.end()) {
        LOG("Resource %llu already registered, skipping duplicate registration", uuid);
        return;  
    }

    resources[uuid] = resource;
    resource->AddReference();

    LOG("Registered resource (UUID: %llu, Name: %s)", uuid, resource->GetName().c_str());
}

std::shared_ptr<Resource> ResourceManager::GetResourceByUUID(VroomUUID uuid) {
    auto it = resources.find(uuid);
    return (it != resources.end()) ? it->second : nullptr;
}

void ResourceManager::AddReference(VroomUUID uuid) {
    auto resource = GetResourceByUUID(uuid);
    if (resource) {
        resource->AddReference();
    }
}

void ResourceManager::RemoveReference(VroomUUID uuid) {
    auto resource = GetResourceByUUID(uuid);
    if (resource) {
        resource->RemoveReference();
    }
}

void ResourceManager::CleanupUnusedResources() {
    int cleaned = 0;

    for (auto it = resources.begin(); it != resources.end(); ) {
        if (it->second->GetReferenceCount() == 0) {
            LOG("Cleaning up unused resource: %llu", it->first);
            it->second->FreeMemory();
            it = resources.erase(it);
            cleaned++;
        }
        else {
            ++it;
        }
    }

    if (cleaned > 0) {
        LOG("Cleaned up %d unused resources", cleaned);
    }
}

bool ResourceManager::LoadResourceFromLibrary(std::shared_ptr<Resource> resource) {
   

    if (!fs->Exists(resource->GetLibraryFilePath())) {
        LOG("ERROR: Library file not found: %s", resource->GetLibraryFilePath());
        return false;
    }

    resource->LoadBin();

    if (resource->GetType() == ResourceType::TEXTURE) {
        auto texture = std::dynamic_pointer_cast<ResourceTexture>(resource);
        if (texture) {
            texture->LoadToGPU();  // ← Should work, data was just loaded
            return texture->isLoadedToGPU;
        }
    }

    if (!resource->IsLoadedToRAM()) {
        LOG("ERROR: Failed to load binary data for resource %llu", resource->GetUUID());
        return false;
    }

    

    return true;
}

bool ResourceManager::LoadResourceToGPU(std::shared_ptr<Resource> resource) {
    if (resource->GetType() == ResourceType::MESH) {
        std::shared_ptr<ResourceMesh> mesh = std::dynamic_pointer_cast<ResourceMesh>(resource);
        if (mesh) {
            mesh->LoadToGPU();
            // Return GPU status (true if successful)
            return mesh->isLoadedToGPU;
        }
    }
    else if (resource->GetType() == ResourceType::TEXTURE) {
        std::shared_ptr<ResourceTexture> texture = std::dynamic_pointer_cast<ResourceTexture>(resource);
        if (texture) {
            texture->LoadToGPU();
            // Return GPU status (true if successful)
            return texture->isLoadedToGPU;
        }
    }

    return false;
}

bool ResourceManager::SaveResourceToLibrary(std::shared_ptr<Resource> resource) {


    if (!fs->Exists(resource->GetLibraryFilePath())) {
        LOG("ERROR: Library file not found: %s", resource->GetLibraryFilePath());
        return false;
    }

    resource->SaveBin();

    //if (!resource->IsLoadedToRAM()) {
    //    LOG("ERROR: Failed to load binary data for resource %llu", resource->GetUUID());
    //    return false;
    //}

    //if (resource->GetType() == ResourceType::MESH) {
    //    std::shared_ptr<ResourceMesh> mesh = std::dynamic_pointer_cast<ResourceMesh>(resource);
    //    if (mesh) {
    //        mesh->LoadToGPU();
    //        // Return GPU status (true if successful)
    //        return mesh->isLoadedToGPU;
    //    }
    //}
    //else if (resource->GetType() == ResourceType::TEXTURE) {
    //    std::shared_ptr<ResourceTexture> texture = std::dynamic_pointer_cast<ResourceTexture>(resource);
    //    if (texture) {
    //        texture->LoadToGPU();
    //        // Return GPU status (true if successful)
    //        return texture->isLoadedToGPU;
    //    }
    //}

    return true;
}

std::shared_ptr<ResourceMesh> ResourceManager::GetPrimitiveMesh(PrimitiveType type) {

    std::shared_ptr<ResourceMesh> mesh = nullptr;

    switch (type) {
    case PrimitiveType::CUBE:
        mesh = CreateCubeMesh();
        break;
    case PrimitiveType::NONE:
        LOG("Invalid Primitive Type");
        return nullptr;
        break;
    default:
        LOG("Unknown Primitive Type requested.");
        return nullptr;
        break;
    }



    if (mesh) mesh->SetUUID(UUIDGen::GenerateUUID());
    
    return mesh;
}

std::shared_ptr<ResourceMesh> ResourceManager::CreateCubeMesh() {
    const glm::vec3 v000(-0.5f, -0.5f, -0.5f);
    const glm::vec3 v001(-0.5f, -0.5f, 0.5f);
    const glm::vec3 v010(-0.5f, 0.5f, -0.5f);
    const glm::vec3 v011(-0.5f, 0.5f, 0.5f);
    const glm::vec3 v100(0.5f, -0.5f, -0.5f);
    const glm::vec3 v101(0.5f, -0.5f, 0.5f);
    const glm::vec3 v110(0.5f, 0.5f, -0.5f);
    const glm::vec3 v111(0.5f, 0.5f, 0.5f);

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;
    std::vector<std::shared_ptr<ResourceTexture>> _textures;

    // Helper arrays for normals and texcoords
    const glm::vec3 normals[6] = {
        glm::vec3(0.0f, 0.0f, 1.0f),   // Front
        glm::vec3(0.0f, 0.0f, -1.0f),  // Back
        glm::vec3(1.0f, 0.0f, 0.0f),   // Right
        glm::vec3(-1.0f, 0.0f, 0.0f),  // Left
        glm::vec3(0.0f, 1.0f, 0.0f),   // Top
        glm::vec3(0.0f, -1.0f, 0.0f)   // Bottom
    };

    const glm::vec2 texCoords[4] = {
        glm::vec2(0.0f, 0.0f), // Bottom left
        glm::vec2(1.0f, 0.0f), // Bottom right
        glm::vec2(1.0f, 1.0f), // Top right
        glm::vec2(0.0f, 1.0f)  // Top left
    };

    // Face definitions: position and normal index
    const glm::vec3 facePositions[6][4] = {
        {v001, v101, v111, v011}, // Front (+Z)
        {v100, v000, v010, v110}, // Back (-Z)
        {v101, v100, v110, v111}, // Right (+X)
        {v000, v001, v011, v010}, // Left (-X)
        {v011, v111, v110, v010}, // Top (+Y)
        {v000, v100, v101, v001}  // Bottom (-Y)
    };


    for (int face = 0; face < 6; face++) {
        for (int vert = 0; vert < 4; vert++) {
            Vertex vertex;
            vertex.Position = facePositions[face][vert];
            vertex.Normal = normals[face];
            vertex.texCoord = texCoords[vert];
            _vertices.push_back(vertex);
        }
    }

    // Build indices (2 triangles per face)
    for (int i = 0; i < 6; i++) {
        int base = i * 4;
        _indices.push_back(base);
        _indices.push_back(base + 1);
        _indices.push_back(base + 2);

        _indices.push_back(base);
        _indices.push_back(base + 2);
        _indices.push_back(base + 3);
    }

    auto cubeMesh = std::make_shared<ResourceMesh>(_vertices, _indices, _textures);
    return cubeMesh;

}

void ResourceManager::ScanAssetsFolder() {
    LOG("Scanning Assets for unmanaged resources.");

    //start recursively search
    std::vector<std::string> assetFiles = fs->IterateAssetsRecursive("Assets");

    //iterate through files 
    for (const auto& assetPath : assetFiles) {
        std::string metaPath = assetPath + ".meta";
        std::string extension = fs->GetExtensionFromPath(assetPath.c_str());

        //skip meta files themselves
        if (extension == "meta") continue;

        //determine resource type
        ResourceType type = DetermineResourceType(assetPath);

        //skip unknown types
        if (type == ResourceType::UNKNOWN || type == ResourceType::SCENE) continue;

        //check if meta file exists
        if (fs->Exists(metaPath.c_str()) && fs->IsMetaValid(metaPath.c_str())) {
            //check if resource needs re-importing
            if (fs->NeedsReimport(metaPath.c_str(), assetPath.c_str())) {
                LOG("MODIFIED ASSET: %s. Re-importing...", assetPath.c_str());
                ImportFile(assetPath, type);
            }
            //otherwise continue
            continue;
        }

        LOG("NEW ASSET FOUND: %s. Auto-importing...", assetPath.c_str());
        ImportFile(assetPath, type);
    }
    LOG("Asset scanning complete.");
}

ResourceType ResourceManager::DetermineResourceType(const std::string& assetsPath) {
    std::string extension = fs->GetExtensionFromPath(assetsPath.c_str());

    if (extension == "png" || extension == "jpg" || extension == "tga" || extension == "dds") {
        return ResourceType::TEXTURE;
    }

    if (extension == "fbx" || extension == "obj") {
        return ResourceType::MESH;
    }

    return ResourceType::UNKNOWN;
}

bool ResourceManager::DeleteResource(VroomUUID uuid) {
    auto it = resources.find(uuid);
    if (it == resources.end()) {
        LOG("WARNING: Tried to delete unmanaged resource UUID %llu", uuid);
        return true;
    }

    std::shared_ptr<Resource> resource = it->second;

    //unload from gpu
    resource->FreeMemory();
    resources.erase(it);

    //delete files
    bool success = true;
    if (!fs->DeleteFile(resource->GetLibraryFilePath())) {
        success = false;
    }
    if (!fs->DeleteFile(resource->GetAssetFilePath())) {
        success = false;
    }
    std::string metaPath = std::string(resource->GetAssetFilePath()) + ".meta";
    if (!fs->DeleteFile(metaPath.c_str())) {
        success = false;
    }

    if (success) LOG("Successfully deleted resource: %s", resource->GetName().c_str());
    else LOG("ERROR: Failed to delete all files for resource: %s", resource->GetName().c_str());
 
    return success;
}

bool ResourceManager::MoveAsset(VroomUUID uuid, const std::string& newAssetPath) {
    //find asset
    std::shared_ptr<Resource> resource = GetResourceByUUID(uuid);
    if (!resource) {
        LOG("ERROR: Cannot move asset - resource not found for UUID %llu", uuid);
        return false;
    }

    //get current path
    std::string oldAssetPath = resource->GetAssetFilePath();
    std::string oldMetaPath = oldAssetPath + ".meta";

    //create the new path
    std::string newMetaPath = newAssetPath + ".meta";

    //move asset file
    if (!fs->MoveFileToNewPath(oldAssetPath.c_str(), newAssetPath.c_str())) {
        return false;
    }

    //move meta file
    if (!fs->MoveFileToNewPath(oldMetaPath.c_str(), newMetaPath.c_str())) {
        fs->MoveFileToNewPath(newAssetPath.c_str(), oldAssetPath.c_str());
        LOG("ERROR: Failed to move meta file for %s. Reverting asset move.", resource->GetName().c_str());
        return false;
    }

    resource->SetAssetFilePath(newAssetPath);

    LOG("Asset moved successfully: %s -> %s", oldAssetPath.c_str(), newAssetPath.c_str());
    return true;
}