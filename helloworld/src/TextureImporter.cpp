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
    std::string normalizedPath = fs->NormalizePath(filePath.c_str());

    LOG("TextureImporter: Importing texture from '%s'", normalizedPath.c_str());

    // Check if file exists
    if (!fs->Exists(normalizedPath.c_str())) {
        LOG("ERROR: Texture file does not exist: %s", normalizedPath.c_str());
        return nullptr;
    }

    // Create Texture resource
    auto texture = std::make_shared<ResourceTexture>();

    // Set paths
    texture->SetAssetFilePath(normalizedPath);
    texture->SetName(fs->GetFileFromPath(normalizedPath.c_str()));

    // Check for existing .meta
    std::string metaPath = normalizedPath + ".meta";
    VroomUUID uuid = 0;

    //if (fs->Exists(metaPath.c_str()) && fs->IsMetaValid(metaPath.c_str())) {
    //    // Load existing UUID
    //    uuid = fs->GetUUIDFromMeta(metaPath.c_str());
    //    texture->SetUUID(uuid);
    //    texture->LoadMeta();  // Load other metadata

    //    LOG("Found existing texture meta (UUID: %llu)", uuid);

    //    // Check if reimport needed
    //    if (!fs->NeedsReimport(metaPath.c_str(), normalizedPath.c_str())) {
    //        // Try to load from Library cache
    //        std::string libraryPath = "Library/Textures/tex_" + std::to_string(uuid) + ".vroomtex";
    //        texture->SetLibraryFilePath(libraryPath);

    //        if (fs->Exists(libraryPath.c_str())) {
    //            LOG("Loading texture from cache: %s", libraryPath.c_str());
    //            texture->LoadBin();      // Load from binary
    //            texture->LoadToGPU();    // Upload to GPU
    //            return texture;
    //        }
    //    }
    //}
    //else {
    // Generate new UUID
    uuid = UUIDGen::GenerateUUID();
    texture->SetUUID(uuid);
    LOG("Generated new UUID for texture: %llu", uuid);
    

    // Import from source file (no cache or needs reimport)
    LOG("Importing texture from source file...");

    // Load image data using stb_image
    int width, height, nChannels;
    unsigned char* data = stbi_load(normalizedPath.c_str(), &width, &height, &nChannels, 0);

    if (!data) {
        LOG("ERROR: Failed to load texture: %s", normalizedPath.c_str());
        LOG("Reason: %s", stbi_failure_reason());
        return nullptr;
    }

    // Set texture data
    texture->texW = width;
    texture->texH = height;
    texture->nChannels = nChannels;
    texture->data = data;  // Texture now owns the data
    texture->mapType = "texture_diffuse";  // Default

    // Generate OpenGL texture and upload to GPU
    glGenTextures(1, &texture->gpu_id);
    glBindTexture(GL_TEXTURE_2D, texture->gpu_id);

    texture->LoadToGPU();  

    // Set library path
    std::string libraryPath = Paths::TEXTURE_LIB_DIR + std::to_string(uuid) + ".vroomtex";
    texture->SetLibraryFilePath(libraryPath);

    // Save to Library cache
    texture->SaveBin();

    // Save/update .meta file
    texture->SaveMeta();

    Application::GetInstance().resourceManager->RegisterResource(texture);

    // Now free the data (already uploaded to GPU and saved to disk)
    if (texture->data) {
        stbi_image_free(texture->data);
        texture->data = nullptr;
    }

    texture->isLoadedToRAM = false; 
    texture->isLoadedToGPU = true;  

    LOG("TextureImporter: Successfully imported '%s' (UUID: %llu, GPU ID: %u, %dx%d)",
        texture->GetName().c_str(), texture->GetUUID(), texture->gpu_id,
        texture->texW, texture->texH);  

    return texture;
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