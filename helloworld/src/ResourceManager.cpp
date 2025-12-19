
#include "ResourceManager.h"
#include "Application.h"
#include "FileSystem.h"
#include "Importer.h"
#include "TextureImporter.h"
#include "MeshImporter.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "SceneManager.h"
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
    fs->CreateDir(Paths::LIB_DIR);
    fs->CreateDir(Paths::MESH_LIB_DIR);
    fs->CreateDir(Paths::TEXTURE_LIB_DIR);

    ReimportMissingFiles();

    DeleteUnusedLibraryFiles();
   
    //scan assets for imgui hierarchy
    ScanAssetsFolder();



    Application::GetInstance().sceneManager->LoadDefaultScene();

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
        /*it->second->AddReference();*/
        return it->second;
    }

    // Determine type from library file extension
    ResourceType type = ResourceType::UNKNOWN;
    std::string libraryPath;

    std::string texturePath = Paths::TEXTURE_LIB_DIR + std::to_string(uuid) + ".vroomtex";
    std::string meshPath = Paths::MESH_LIB_DIR + std::to_string(uuid) + ".vroommesh";
    

    if (fs->Exists(texturePath.c_str())) {
        type = ResourceType::TEXTURE;
        libraryPath = texturePath;
        
    }
    else if (fs->Exists(meshPath.c_str())) {
        type = ResourceType::MESH;
        libraryPath = meshPath;
    }
    else {
        LOG("ERROR: Library file not found for UUID %llu, reimporting", uuid);
        TryReimportResource(uuid, type);

        //TODO: REIMPORT !!
        return nullptr;
    }

    // Create and load from library
    auto resource = CreateResource(type, uuid);
    resource->SetLibraryFilePath(libraryPath);
    if (resource) {
        LoadResourceFromLibrary(resource);
        RegisterResource(resource);
        
        //resource->AddReference();

        return resource;
    }

    return nullptr;
}

bool ResourceManager::TryReimportResource(VroomUUID uuid, ResourceType& outType) {


    LOG("Attempting to reimport resource %llu", uuid);

    // Search for .meta files in Assets that reference this UUID
    std::string assetsPath = Paths::ASSETS_DIR;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath)) {
        
        if (!entry.is_regular_file()) continue;
        std::string path = fs->NormalizePath(entry.path().string().c_str());
        std::string ext = fs->GetExtensionFromPath(path.c_str());
        if (ext == "fbx" || ext == "obj") {
            //Find the meta file
            std::string metaPath = path + ".meta";
            metaPath = fs->NormalizePath(metaPath.c_str());
            if (!fs->Exists(metaPath.c_str())) continue;
            nlohmann::json modelMeta = fs->LoadJSON(metaPath.c_str());

            if (modelMeta.contains("meshes")) 
            {
                for (auto& meshEntry : modelMeta["meshes"]) 
                {
                    if (meshEntry.contains("meshUUID")){

                        if (meshEntry["meshUUID"].get<VroomUUID>() == uuid) {
                            // Resource is a mesh! Reimport the model
                            std::string modelPath = path;
                            modelPath = fs->NormalizePath(modelPath.c_str());
                            //modelPath = modelPath.substr(0, modelPath.length() - 5); // Remove ".meta"

                            LOG("Found source model: %s", modelPath.c_str());
                            LOG("Reimporting to regenerate mesh UUID %llu", uuid);

                            // Reimport (importers already handle looking at the meta!)
                            Application::GetInstance().sceneManager->GetActiveScene()->ImportModel(modelPath, &modelMeta, false);

                            outType = ResourceType::MESH;
                            return true;
                        }

                        if (meshEntry.contains("meshTextures")) {
                            for (auto& texEntry : meshEntry["meshTextures"]) {
                                if (texEntry.contains("texUUID")) {
                                    if (texEntry["texUUID"].get<VroomUUID>() == uuid) {
                                        //resource is a texture being used by a model! Reimport texture

                                        //two possibilities: either it's in the same path as the model, or it's in the textures folder
                                        std::string texturePath1 = path; //same path as model
                                        std::string texturePath2 = Paths::TEXTURE_ASSETS_DIR + fs->GetFileFromPath(path.c_str()); //textures folder
                                        std::string foundTexturePath;

                                        if (fs->Exists(texturePath1.c_str())) {
                                            LOG("Texture found in the same directory as the source model: %s", texturePath1.c_str());
                                            foundTexturePath = texturePath1;
                                        }
                                        else if (fs->Exists(texturePath2.c_str())) {
                                            LOG("Texture found in the textures folder : %s", texturePath2.c_str());
                                            foundTexturePath = texturePath2;
                                        }
                                        else {
                                            std::string checkersPath = Application::GetInstance().importer.get()->defaultTexDir;
                                            LOG("Texture directory not found, will default to checkers texture: %s", checkersPath.c_str());
                                            foundTexturePath = checkersPath;
                                        }

                                        outType = ResourceType::TEXTURE;
                                        LOG("Reimporting to regenerate texture UUID %llu", uuid);
                                        Application::GetInstance().resourceManager.get()->ImportFile(foundTexturePath, outType);
                                        return true;
                                        
                                    }
                                }
                            }
                        }
                        

                    }

                }
            }
        }

        
        if (ext == "png" || ext == "dds" || ext == "jpg" || ext == "tga") {
            std::string texMetaPath = path + ".meta";
            texMetaPath = fs->NormalizePath(texMetaPath.c_str());

            nlohmann::json texMeta = fs->LoadJSON(texMetaPath.c_str());
            if (!fs->Exists(texMetaPath.c_str())) continue;
               
            //check if uuid is in meta
            if (texMeta.contains("uuid") && texMeta["uuid"].get<VroomUUID>() == uuid) {
                std::string texturePath = path;
                texturePath = fs->NormalizePath(texturePath.c_str());
                //texturePath = texturePath.substr(0, texturePath.length() - 5); // Remove ".meta"

                LOG("Found source texture: %s", texturePath.c_str());
                LOG("Reimporting texture UUID %llu", uuid);

                // Reimport texture
                Application::GetInstance().importer.get()->textureImporter->Import(texturePath.c_str());

                outType = ResourceType::TEXTURE;
                return true;
            }


                
        }
        
    }

    LOG("ERROR: Could not find source file for UUID %llu", uuid);
    return false;
}

std::shared_ptr<Resource> ResourceManager::RequestResource(const std::string& assetsPath) {

    std::string normalizedPath = fs->NormalizePath(assetsPath.c_str());
    std::string metaPath = normalizedPath + ".meta";
    VroomUUID resUUID = 0;

    // --- 1. GET UUID (from meta or by fresh import) ---
    if (fs->Exists(metaPath.c_str())) {
        LOG(".meta file found for %s, loading via UUID.", normalizedPath.c_str());
        resUUID = fs->GetUUIDFromMeta(metaPath.c_str());
    }
    else {
        // If meta is missing, import fresh to generate UUID/library file
        LOG("No .meta file found for %s, importing fresh.", normalizedPath.c_str());

        ResourceType type = DetermineResourceType(normalizedPath);
        resUUID = ImportFile(normalizedPath, type); // ImportFile returns the new UUID
    }

    // Handle import failure
    if (resUUID == 0) {
        LOG("ERROR: Failed to obtain UUID for path: %s", normalizedPath.c_str());
        return nullptr;
    }

   
    auto resource = RequestResource(resUUID);

    if (resource) {
        resource->AddReference();
    }

    return resource;
}

VroomUUID ResourceManager::ImportFile(const std::string& assetsPath, ResourceType type, bool addToScene) {
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
        Application::GetInstance().importer.get()->modelImporter->ImportScene(assetsPath.c_str(), addToScene);
        
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
    //resource->AddReference(); <-- REGISTERING RESOURCE != OWNERSHIP !!

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


void ResourceManager::ReimportMissingFiles() {
    std::string assetsPath = Paths::ASSETS_DIR;
    if (!fs->Exists(assetsPath.c_str())) {
        LOG("Assets folder not found, creating...");
        fs->CreateDir(assetsPath.c_str());
        return;
    }

    int scanned = 0;
    int generated = 0;
    int reimported = 0;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() == ".meta") continue;

        std::string assetPath = fs->NormalizePath(entry.path().string().c_str());
        std::string extension = fs->GetExtensionFromPath(assetPath.c_str());

        // Only process known asset types
        if (extension != "png" && extension != "jpg" && extension != "tga" &&
            extension != "dds" && extension != "fbx" && extension != "obj") {
            continue;
        }

        scanned++;
        std::string metaPath = assetPath + ".meta";
        bool needsImport = false;

        // Case 1: .meta doesn't exist
        if (!fs->Exists(metaPath.c_str())) {
            LOG("Missing .meta for: %s - will reimport to generate", assetPath.c_str());
            needsImport = true;
            generated++;
        }
        // Case 2: .meta is invalid/corrupted
        else if (!fs->IsMetaValid(metaPath.c_str())) {
            LOG("Invalid .meta for: %s - will reimport to regenerate", assetPath.c_str());
            needsImport = true;
            generated++;
        }
        // Case 3: Asset was modified (different timestamp)
        else if (fs->NeedsReimport(metaPath.c_str(), assetPath.c_str())) {
            LOG("Asset modified, needs reimport: %s", assetPath.c_str());
            needsImport = true;
            reimported++;

            // Delete old Library files before reimporting
            /*DeleteUnusedLibraryFiles((metaPath, extension);)*/
        }

        // Reimport if needed
        if (needsImport) {
            ResourceType type = DetermineResourceType(assetPath);

            if (type == ResourceType::UNKNOWN) {
                LOG("WARNING: Skipping unknown file type: %s", assetPath.c_str());
                continue;
            }

            VroomUUID uuid = ImportFile(assetPath, type, false);

            if (type == ResourceType::TEXTURE && uuid == 0) {
                LOG("ERROR: Failed to import texture: %s", assetPath.c_str());
            }
            else if (type == ResourceType::MESH) {
                LOG("Model imported successfully: %s", assetPath.c_str());
            }
        }
        
    }

    LOG("========================================");
    LOG("Asset scan complete:");
    LOG("  - %d assets scanned", scanned);
    LOG("  - %d .meta files generated", generated);
    LOG("  - %d assets reimported", reimported);
    LOG("========================================");
}

void ResourceManager::DeleteUnusedLibraryFiles() {
    LOG("Cleaning up orphaned library files...");

    std::unordered_set<VroomUUID> validUUIDs;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(std::string(Paths::ASSETS_DIR))) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();
        path = fs->NormalizePath(path.c_str());

        if (fs->GetExtensionFromPath(path.c_str()) != "meta") continue;

        // Add try-catch for JSON parsing
        try {
            LOG("Reading .meta file: %s", path.c_str());
            nlohmann::json meta = fs->LoadJSON(path.c_str());
            LOG("Successfully parsed .meta");

            if (meta.empty() || meta.is_null()) {
                LOG("WARNING: Empty or null .meta file: %s", path.c_str());
                continue;
            }

            // Collect UUIDs from MODEL metas
            if (meta.contains("meshes")) {
                for (const auto& mesh : meta["meshes"]) {
                    if (mesh.contains("meshUUID")) {
                        validUUIDs.insert(mesh["meshUUID"].get<VroomUUID>());
                    }

                    if (mesh.contains("meshTextures")) {
                        for (const auto& tex : mesh["meshTextures"]) {
                            if (tex.contains("texUUID")) {
                                validUUIDs.insert(tex["texUUID"].get<VroomUUID>());
                            }
                        }
                    }
                }
            }
            // Collect UUIDs from STANDALONE RESOURCE metas
            else if (meta.contains("uuid")) {
                validUUIDs.insert(meta["uuid"].get<VroomUUID>());
            }
            else {
                LOG("WARNING: .meta file has no recognizable structure: %s", path.c_str());
            }

        }
        catch (const nlohmann::json::parse_error& e) {
            LOG("ERROR: Failed to parse .meta file: %s", path.c_str());
            LOG("Parse error: %s", e.what());
            LOG("Skipping this file...");
            continue;
        }
        catch (const std::exception& e) {
            LOG("ERROR: Exception while processing .meta file: %s", path.c_str());
            LOG("Error: %s", e.what());
            continue;
        }
    }

    LOG("Found %zu valid UUIDs in Assets metas", validUUIDs.size());

    int deletedFiles = 0;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(std::string(Paths::LIB_DIR))) {
            if (!entry.is_regular_file()) continue;

            std::string filename = entry.path().stem().string();

            // Add validation for filename
            if (filename.empty()) {
                LOG("WARNING: Empty filename in Library, skipping");
                continue;
            }

            try {
                VroomUUID uuid = std::stoull(filename);

                if (validUUIDs.find(uuid) == validUUIDs.end()) {
                    std::filesystem::remove(entry.path());
                    deletedFiles++;
                    LOG("Deleted orphaned library file: %s", entry.path().string().c_str());
                }
            }
            catch (const std::exception& e) {
                LOG("WARNING: Invalid UUID filename in Library: %s", filename.c_str());
                continue;
            }
        }
    }
    catch (const std::exception& e) {
        LOG("ERROR: Exception while cleaning Library: %s", e.what());
    }

    LOG("Unused Library Files Clean Up Complete: %d files deleted", deletedFiles);
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



    if (mesh) {
        mesh->SetUUID(UUIDGen::GenerateUUID());
        RegisterResource(mesh);

       
        /*AddReference(mesh->GetUUID());*/ 
        //we increase later, when adding the mesh renderer component

        LOG("Created primitive (UUID: %llu, refCount: %d)", mesh->GetUUID(), mesh->GetReferenceCount());
    }
    
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
    std::vector<std::string> assetFiles = fs->IterateAssetsRecursive(Paths::ASSETS_DIR);

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
        return ResourceType::SCENE;
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

bool ResourceManager::DeleteFileFromLibrary(const std::string filePath) {
    FileSystem* fs = Application::GetInstance().fileSystem.get();
    if (fs->DeleteFile(filePath.c_str())) {
        LOG("Resource file successfully deleted from %s", filePath.c_str());
        return true;
    }
    LOG("ERROR: Couldn't delete resource file at %s", filePath.c_str());
    return false;
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