#pragma once
#include <memory>
#include <string>
#include "Textures.h"

// Forward declaration
//class Texture;

class TextureImporter {
public:
    
    static std::shared_ptr<Texture> Import(const std::string& filePath);

    // Overload for compatibility with old API (directory + filename)
    static std::shared_ptr<Texture> Import(const std::string& directory, const char* filename);

};