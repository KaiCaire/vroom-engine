#pragma once

#include "Module.h"
#include "UUID.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include <unordered_map>
#include <memory>
#include <string>

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
    std::shared_ptr<Resource> RequestResource(VroomUUID uuid);

    // Request a resource by file path (checks cache, imports if needed)
    std::shared_ptr<Resource> RequestResource(const std::string& assetsPath);

    // Import a new file from disk
    VroomUUID ImportFile(const std::string& assetsPath, ResourceType type);

    // Create a new resource (used by importers)
    std::shared_ptr<Resource> CreateResource(ResourceType type, VroomUUID uuid = 0);

    // Register a resource (adds to cache)
    void RegisterResource(std::shared_ptr<Resource> resource);

    // Find resource by UUID
    std::shared_ptr<Resource> FindResource(VroomUUID uuid);

    // Reference counting
    void AddReference(VroomUUID uuid);
    void RemoveReference(VroomUUID uuid);

    // Cleanup unused resources (refCount == 0)
    void CleanupUnusedResources();

    ResourceMesh* CreateCubeMesh();

    ResourceMesh* GetPrimitiveMesh(PrimitiveType type);

    //asset viewer
    void ScanAssetsFolder();
    ResourceType DetermineResourceType(const std::string& assetsPath);
    const std::unordered_map<VroomUUID, std::shared_ptr<Resource>>& GetAllResources() const {
        return resources;
    }
    

private:
    // All loaded resources indexed by UUID
    std::unordered_map<VroomUUID, std::shared_ptr<Resource>> resources;

    // Helper to load resource from Library
    bool SaveResourceToLibrary(std::shared_ptr<Resource> resource);
    bool LoadResourceFromLibrary(std::shared_ptr<Resource> resource);
    bool LoadResourceToGPU(std::shared_ptr<Resource> resource);
    

    FileSystem* fs;
};
