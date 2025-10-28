#include "Textures.h"
#include "Application.h"
#include "Render.h"
#include "Log.h"
#include <string>
#include <algorithm>
#include <iostream>
#include "FileSystem.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

Texture::Texture() : Module()
{
	name = "textures";
    id = -1;
    mapType = "";
    path = "";
}

// Destructor
Texture::~Texture()
{
}

// Called before render is available
bool Texture::Awake()
{
	
	bool ret = true;

	return ret;
}

// Called before the first frame
bool Texture::Start()
{
	
	bool ret = true;
	return ret;
}

// Called before quitting
bool Texture::CleanUp()
{
	
	return true;
}


uint Texture::TextureFromFile(const string directory, const char* filename) {

    std::string editedDirectory = directory;

    std::replace(editedDirectory.begin(), editedDirectory.end(), '\\', '/');
    editedDirectory = editedDirectory.substr(0, editedDirectory.find_last_of("/") + 1);
    
    std::string filePath = editedDirectory + '/' + filename;

    /*unsigned int textureID;*/
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    int width, height, nChannels;
    
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nChannels, 0);
    if (!data)
    {
        cout << "Failed to load texture: " << filePath << endl;
        cout << "Reason: " << stbi_failure_reason() << endl;
       
    }
    else 
    {
        GLenum format;
        switch (nChannels) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            format = GL_RGB;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);


        //Setting various texture parameters:
        //Texture-Wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //S = X axis in texCoords
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //T = Y axis texCoords

        //Filtering mode- -> GL_NEAREST = blocky pattern (default) || GL_LINEAR = smoother pattern
        // can be set separately for minifying or magnifying operations:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // mip maps are only implemented in downscaling! don't filter mipmaps with MAG_FILTER 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    
    path = filePath;
 
    stbi_image_free(data);

    return id;
}



