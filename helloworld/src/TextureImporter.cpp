#include "TextureImporter.h"
#include "Textures.h"
#include "Application.h"
#include "FileSystem.h"
#include "OpenGL.h"
#include "Log.h"
#include "stb_image.h"
#include <iostream>

std::shared_ptr<Texture> TextureImporter::Import(const std::string& filePath) {
    if (filePath.empty()) {
        LOG("ERROR: TextureImporter received empty file path");
        return nullptr;
    }

    LOG("TextureImporter: Loading texture from '%s'", filePath.c_str());

    // Create Texture resource
    auto texture = std::make_shared<Texture>();

    // Normalize path using FileSystem
    FileSystem* fs = Application::GetInstance().fileSystem.get();
    std::string normalizedPath = fs->NormalizePath(filePath.c_str());

    // Generate OpenGL texture
    glGenTextures(1, &texture.get()->id);
    glBindTexture(GL_TEXTURE_2D, texture.get()->id);

    // Load image data
    int width, height, nChannels;
    unsigned char* data = stbi_load(normalizedPath.c_str(), &width, &height, &nChannels, 0);

    if (!data) {
        std::cout << "Failed to load texture: " << normalizedPath << std::endl;
        std::cout << "Reason: " << stbi_failure_reason() << std::endl;
        LOG("Failed to load texture: %s", normalizedPath.c_str());
        LOG("Reason: %s", stbi_failure_reason());
        return nullptr;
    }

    texture.get()->texW = width;
    texture.get()->texH = height;

    // Determine format
    GLenum format = GL_RGB;

    switch (nChannels) {
    case 1: format = GL_RED; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    default: format = GL_RGB;
    }

    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Set resource metadata using FileSystem
    std::string filename = fs->GetFileFromPath(normalizedPath.c_str());

    texture.get()->path = normalizedPath;
    texture.get()->SetFilePath(normalizedPath);
    texture.get()->SetName(filename);
    texture.get()->mapType = "texture_diffuse";  // Default, can be changed later
    texture.get()->isLoaded = true;

    stbi_image_free(data);

    // TODO: Generate or retrieve UUID from meta file
    // texture->SetUID(generatedUID);

    LOG("TextureImporter: Successfully imported texture '%s' (ID: %d, %dx%d)",
        filename.c_str(), texture.get()->id, texture.get()->texW, texture.get()->texH);

    return texture;
}

std::shared_ptr<Texture> TextureImporter::Import(const std::string& directory,
    const char* filename) {
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