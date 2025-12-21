#pragma once

#include "Module.h"
#include "UUID.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include <unordered_map>
#include <memory>
#include <string>

#define UUID_CUBE   100ULL
#define UUID_PLANE  101ULL
#define UUID_SPHERE 102ULL

enum class ResourceType;

enum class PrimitiveType {
    CUBE,
    SPHERE,
    PLANE,
    CYLINDER,
    NONE
};




class ResourceManager : public Module {
public:
    ResourceManager();
    ~ResourceManager();

    bool Start() override;
    bool CleanUp() override;

    // Request a resource by UUID (loads if not loaded)
    std::shared_ptr<Resource> RequestResource(VroomUUID uuid, bool allowRetry = true);

    // Request a resource by file path (checks cache, imports if needed)
    std::shared_ptr<Resource> RequestResource(const std::string& assetsPath, const std::string& sourcePath = "");

    // Import a new file from disk
    VroomUUID ImportFile(const std::string& assetsPath, ResourceType type, const std::string& sourcePath = "", bool addToScene = true);

    // Create a new resource (used by importers)
    std::shared_ptr<Resource> CreateResource(ResourceType type, VroomUUID uuid = 0);

    // Register a resource (adds to cache)
    void RegisterResource(std::shared_ptr<Resource> resource);

    // Find resource by UUID
    std::shared_ptr<Resource> GetResourceByUUID(VroomUUID uuid);

    // Reference counting
    void AddReference(VroomUUID uuid);
    void RemoveReference(VroomUUID uuid);

    // Cleanup unused resources (refCount == 0)
    void DeleteUnusedLibraryFiles();
    void ReimportMissingFiles();
    void InitializePrimitives();

    std::shared_ptr<ResourceMesh> CreateCubeMesh();


    std::shared_ptr<ResourceMesh> GetPrimitiveMesh(PrimitiveType type);

    //asset viewer
    void ScanAssetsFolder();
    ResourceType DetermineResourceType(const std::string& assetsPath);
    const std::unordered_map<VroomUUID, std::shared_ptr<Resource>>& GetAllResources() const {
        return resources;
    }
    bool DeleteResource(VroomUUID uuid);
    bool MoveAsset(VroomUUID uuid, const std::string& newAssetPath);
    bool LoadResourceToGPU(std::shared_ptr<Resource> resource);
    bool DeleteFileFromLibrary(const std::string filePath);
    


    //DEFAULTS:
    std::string checkersTexDir;
    std::shared_ptr<ResourceTexture> whiteDefault;
    std::shared_ptr<ResourceTexture> blackDefault;
    

private:
    // All loaded resources indexed by UUID
    std::unordered_map<VroomUUID, std::shared_ptr<Resource>> resources;

    // Helper to load resource from Library
    bool SaveResourceToLibrary(std::shared_ptr<Resource> resource);
    bool LoadResourceFromLibrary(std::shared_ptr<Resource> resource);
    bool TryReimportResource(VroomUUID uuid, ResourceType& outType);
    

    FileSystem* fs;
};
