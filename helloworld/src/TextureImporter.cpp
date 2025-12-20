#include "TextureImporter.h"
#include "ResourceTexture.h"
#include "Application.h"
#include "FileSystem.h"
#include "OpenGL.h"
#include "Log.h"
#include "stb_image.h"
#include <iostream>
#include "ResourceManager.h"


std::shared_ptr<ResourceTexture> TextureImporter::Import(const std::string& filePath) {
    if (filePath.empty()) {
        LOG("ERROR: TextureImporter received empty file path");
        return nullptr;
    }

    FileSystem* fs = Application::GetInstance().fileSystem.get();
    ResourceManager* rm = Application::GetInstance().resourceManager.get();
    std::string normalizedPath = fs->NormalizePath(filePath.c_str());

    LOG("TextureImporter: Importing texture from '%s'", normalizedPath.c_str());

    // Check if file exists
    if (!fs->Exists(normalizedPath.c_str())) {
        LOG("ERROR: Texture file does not exist: %s", normalizedPath.c_str());
        return nullptr;
    }

    // determine uuid & check for existing .meta
    std::string metaPath = normalizedPath + ".meta";
    VroomUUID uuid = 0;

    if (fs->Exists(metaPath.c_str()) && fs->IsMetaValid(metaPath.c_str())) {
        // Load existing UUID
        uuid = fs->GetUUIDFromMeta(metaPath.c_str());
        LOG("TextureImporter: Found existing meta for %s (UUID: %llu)", normalizedPath.c_str(), uuid);
    }
    else {
        uuid = UUIDGen::GenerateUUID();
        LOG("TextureImporter: No meta found. Generated new UUID: %llu", uuid);
    }

    //check resource registry to see if it's already loaded in RAM:
    auto existing = rm->GetResourceByUUID(uuid);
    if (existing) {
        LOG("TextureImporter: Resource %llu already in RAM. Returning existing pointer.", uuid);
        return std::dynamic_pointer_cast<ResourceTexture>(existing);
    }

    //check library to see if it's already loaded on disk
    std::string libraryPath = Paths::TEXTURE_LIB_DIR + std::to_string(uuid) + ".vroomtex";


    if (fs->Exists(libraryPath.c_str())) {
        LOG("TextureImporter: Found binary in Library. Loading optimized file...");

        // Create Texture resource
        auto texture = std::make_shared<ResourceTexture>();
        texture->SetUUID(uuid);
        // Set paths
        texture->SetAssetFilePath(normalizedPath);
        texture->SetName(fs->GetFileFromPath(normalizedPath.c_str()));

        //load & register
        texture->LoadBin();
        texture->LoadToGPU();
        rm->RegisterResource(texture);

        return texture;
    }
    
    // If we are here, the .vroomtex is MISSING. We must re-process the JPG/PNG.
    LOG("Re-importing texture from source...");

    int width, height, nChannels;
    unsigned char* data = stbi_load(normalizedPath.c_str(), &width, &height, &nChannels, 0);

    if (!data) {
        LOG("ERROR: stbi_load failed for: %s", normalizedPath.c_str());
        return nullptr;
    }

    //create new texture
    auto texture = std::make_shared<ResourceTexture>();
    texture->SetUUID(uuid);
    texture->SetAssetFilePath(normalizedPath);
    texture->SetLibraryFilePath(libraryPath);
    texture->SetName(fs->GetFileFromPath(normalizedPath.c_str()));

    // Set texture data
    texture->texW = width;
    texture->texH = height;
    texture->nChannels = nChannels;
    texture->data = data;
    texture->mapType = "texture_diffuse";

    // SAVE & UPLOAD
    texture->LoadToGPU();
    texture->SaveBin();  // This heals the Library folder if the file was missing
    texture->SaveMeta(); // This ensures the .meta file is synced
    rm->RegisterResource(texture);

    // CLEANUP RAM
    stbi_image_free(data);
    texture->data = nullptr;
    texture->isLoadedToRAM = false;

    LOG("Texture Importer failed, returning nullptr");
    return nullptr; //really shouldn't happen but just in case
}

std::shared_ptr<ResourceTexture> TextureImporter::Import(const std::string& directory, const char* filename) {
    // Normalize directory using FileSystem
    FileSystem* fs = Application::GetInstance().fileSystem.get();
    std::string normalizedDir = fs->NormalizePath(directory.c_str());

    // Ensure directory ends with /
    if (!normalizedDir.empty() && normalizedDir.back() != '/') {
        normalizedDir += '/';
    }

    std::string fullPath = normalizedDir + filename;

    // Call the main import function
    return Import(fullPath);
}