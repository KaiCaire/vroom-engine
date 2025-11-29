#pragma once
#include "Resource.h"
#include <string>

// Forward declarations
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int uint;

// Texture is now a Resource
class Texture : public Resource 
{
public:
    // Texture data
    unsigned int id;        // OpenGL texture ID
    std::string mapType;    // e.g., "texture_diffuse", "texture_specular"
    std::string path;       // Full file path (kept for backward compatibility)
    int texW, texH;         // Dimensions
    
    // Constructor
    Texture();
    ~Texture();
    
    // OLD method - kept for backward compatibility (deprecated)
    /*uint TextureFromFile(const std::string directory, const char* filename);*/
    
    // Resource interface implementation
    bool LoadToMemory() override;
    void UnloadFromMemory() override;


};


	
	


