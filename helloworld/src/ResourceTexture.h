#pragma once
#include "Resource.h"
#include <string>

// Forward declarations
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int uint;


enum Format {
    COLOR_INDEX,
    RBG,
    RBGA,
    BRG,
    BRGA,
    LUMINANCE,
    UNKNOWN_FORMAT
};

// Texture is now a Resource
class ResourceTexture : public Resource 
{
public:
    // Texture data
    unsigned int id;        // OpenGL texture ID
    std::string mapType;    // e.g., "texture_diffuse", "texture_specular"
    std::string path;       // Full file path (kept for backward compatibility)
    int texW, texH;         // Dimensions
    uint nChannels;         // number of channels (RGB/RGBA)
    unsigned char* data = nullptr; //contains raw pixel info (RBG/RGBA values for each pixel)
 
    // Constructor
    ResourceTexture();
    ~ResourceTexture();
    

    void SaveBin() override;
    void LoadBin() override;
    void SaveMeta() const override;
    void LoadMeta() override;

    void FreeMemory();

    void LoadToGPU();
    void UnloadFromGPU();

    uint gpu_id = 0;
    bool isLoadedToGPU = false;

    Format format = Format::UNKNOWN_FORMAT;


};


	
	


