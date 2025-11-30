#pragma once
#include <memory>
#include <string>
#include "ResourceTexture.h"

// Forward declaration
//class ResourceTexture;

class TextureImporter {
public:
    
    static std::shared_ptr<ResourceTexture> Import(const std::string& filePath);

    // Overload for compatibility with old API (directory + filename)
    static std::shared_ptr<ResourceTexture> Import(const std::string& directory, const char* filename);

};