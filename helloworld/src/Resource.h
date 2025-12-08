#pragma once
#include <string>
#include "UUID.h"

#include "Application.h"
#include "FileSystem.h"


namespace Paths {

    inline const char* ASSETS_DIR = "../Assets";
    inline const char* MODEL_ASSETS_DIR = "../Assets/Models";
    inline const char* TEXTURE_ASSETS_DIR = "../Assets/Textures";
    inline const char* SCENE_ASSETS_DIR = "../Assets/Scenes";

    inline const char* LIB_DIR = "../Library";
    inline const char* MESH_LIB_DIR = "../Library/Meshes/";
    inline const char* TEXTURE_LIB_DIR = "Library/Textures/";    
}


typedef unsigned int uint;


enum class ResourceType {
    UNKNOWN = 0,
    MESH,
    SCENE,
    TEXTURE,
    MATERIAL,
    SHADER,
    AUDIO,  
};

class Resource {
public:
    
    Resource(ResourceType type);
    virtual ~Resource() = default;

    // Prevent copying of Resources
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    // Getters
    VroomUUID GetUUID() const { return uuid; }
    std::string GetName() const { return name; }
    //std::string GetFilePath() const { return filePath; }
    ResourceType GetType() const { return type; }
    bool IsLoadedToRAM() const { return isLoadedToRAM; }

    int GetReferenceCount() const { return referenceCount; }

    // Setters
    void SetUUID(VroomUUID id) { uuid = id; }
    void SetName(const std::string& n) { name = n; }
    /*void SetFilePath(const std::string& path) { filePath = path; }*/

    const char* GetAssetFilePath() const { return assetsPath.c_str(); }
    const char* GetLibraryFilePath() const { return libraryPath.c_str(); }

    void SetAssetFilePath(const std::string& path) { assetsPath = path; }
    void SetLibraryFilePath(const std::string& path) { libraryPath = path; }


    // Reference counting
    void AddReference() { referenceCount++; }
    void RemoveReference() {
        if (referenceCount > 0) referenceCount--;
    }

    bool isLoadedToRAM = false; // Is data loaded in RAM?
    bool isLoadedToGPU = false;


    //virtual bool LoadToMemory() = 0;
    //virtual void UnloadFromMemory() = 0;
    

    virtual void SaveBin() = 0; //write binary data to Library
    virtual void LoadBin() = 0; //read binary data from Library
    virtual void FreeMemory() = 0; //unload resource from memory, freeing RAM

    virtual void SaveMeta() const = 0; //.meta to Assets
    virtual void LoadMeta() = 0; //.meta from Assets

    static std::string GetTypeString(ResourceType type);

protected:
    VroomUUID uuid;
    std::string name;

    std::string assetsPath;
    std::string libraryPath;

    ResourceType type;
    uint referenceCount;

   

    FileSystem* fs;
};