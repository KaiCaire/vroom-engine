#include "Textures.h"
#include "Application.h"
#include "Render.h"
#include "OpenGL.h"
#include "Log.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using namespace std;

Texture::Texture(): Resource(ResourceType::TEXTURE) {
    //name = "texture";
    //id = 0;  // Changed from -1 to 0 (OpenGL convention for uninitialized)
    //mapType = "";
    //path = "";
    //texW = 0;
    //texH = 0;
    //isLoaded = false;
}

Texture::~Texture() {
    UnloadFromMemory();
}

//uint Texture::TextureFromFile(const std::string directory, const char* filename) {
//    std::string editedDirectory = directory;
//    std::replace(editedDirectory.begin(), editedDirectory.end(), '\\', '/');
//    editedDirectory = editedDirectory.substr(0, editedDirectory.find_last_of("/") + 1);
//
//    std::string filePath;
//    if (editedDirectory.empty() || editedDirectory[editedDirectory.size() - 1] != '/') {
//        filePath = editedDirectory + '/' + filename;
//    }
//    else {
//        filePath = editedDirectory + filename;
//    }
//
//    // Generate OpenGL texture
//    glGenTextures(1, &id);
//    glBindTexture(GL_TEXTURE_2D, id);
//
//    int width, height, nChannels;
//    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nChannels, 0);
//    texW = width;
//    texH = height;
//
//    if (!data) {
//        cout << "Failed to load texture: " << filePath << endl;
//        cout << "Reason: " << stbi_failure_reason() << endl;
//        LOG("Failed to load texture: %s", filePath.c_str());
//        LOG("Reason: %s", stbi_failure_reason());
//        isLoaded = false;
//        return 0;
//    }
//    else {
//        GLenum format;
//        switch (nChannels) {
//        case 1:
//            format = GL_RED;
//            break;
//        case 3:
//            format = GL_RGB;
//            break;
//        case 4:
//            format = GL_RGBA;
//            break;
//        default:
//            format = GL_RGB;
//        }
//
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        // Setting various texture parameters:
//        // Texture-Wrap
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // S = X axis in texCoords
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); // T = Y axis texCoords
//
//        // Filtering mode -> GL_NEAREST = blocky pattern (default) || GL_LINEAR = smoother pattern
//        // can be set separately for minifying or magnifying operations:
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        // mip maps are only implemented in downscaling! don't filter mipmaps with MAG_FILTER 
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//
//        isLoaded = true;
//    }
//
//    path = filePath;
//    this->filePath = filePath;  // Set Resource base class member
//    this->name = filename;       // Set Resource base class member
//
//    stbi_image_free(data);
//    return id;
//}

bool Texture::LoadToMemory() {
    // Already loaded during TextureFromFile
    return isLoaded;
}

void Texture::UnloadFromMemory() {
    if (id != 0) {
        glDeleteTextures(1, &id);
        id = 0;
    }
    isLoaded = false;
}