#pragma once
#include <string>
#include "UUID.h"

enum class ResourceType {
    UNKNOWN = 0,
    MESH,
    TEXTURE,
    MATERIAL,
    SHADER,
    AUDIO
};

class Resource {
protected:
    VroomUUID uuid;
    std::string name;
    std::string filePath;
    ResourceType type;
    
    int referenceCount;

public:
    Resource(ResourceType type)
        : uuid(0), type(type), isLoaded(false), referenceCount(0) {
    }

    virtual ~Resource() = default;
    bool isLoaded;

    // Prevent copying of Resources
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    // Getters
    VroomUUID GetUUID() const { return uuid; }
    std::string GetName() const { return name; }
    std::string GetFilePath() const { return filePath; }
    ResourceType GetType() const { return type; }
    bool IsLoaded() const { return isLoaded; }
    int GetReferenceCount() const { return referenceCount; }

    // Setters
    void SetUUID(VroomUUID id) { uuid = id; }
    void SetName(const std::string& n) { name = n; }
    void SetFilePath(const std::string& path) { filePath = path; }

    // Reference counting
    void AddReference() { referenceCount++; }
    void RemoveReference() {
        if (referenceCount > 0) referenceCount--;
    }

    // Pure virtual - must be implemented by derived classes
    virtual bool LoadToMemory() = 0;
    virtual void UnloadFromMemory() = 0;
};