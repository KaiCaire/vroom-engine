#include "ResourceTexture.h"
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

ResourceTexture::ResourceTexture(): Resource(ResourceType::TEXTURE), id(0), texW(0), texH(0), nChannels(0) {
}

ResourceTexture::~ResourceTexture() {
    FreeMemory();
    UnloadFromGPU();
}

void ResourceTexture::SaveBin() {
    //create directory if it doesn't exist yet:

    std::string filePath = libraryPath;
    std::string directory = fs->GetDirFromPath(filePath.c_str());
    if (!directory.empty() && fs->Exists(directory.c_str())) {
        fs->CreateDir(directory.c_str());
       
    }

    //asset path and map type string lengths
    uint pathLen = (uint)assetsPath.length();
    uint typeLen = (uint)mapType.length();

    // Calculate total size: UUID + W + H + Channels + PathLen + PathChars + TypeLen + TypeChars + PixelData
    uint headerSize = sizeof(VroomUUID) + (sizeof(uint) * 3); // uuid + width + height + channels
    uint stringInfoSize = (sizeof(uint) * 2) + pathLen + typeLen;
    uint dataSize = texW * texH * nChannels;
    uint totalSize = headerSize + stringInfoSize + dataSize;

    //Create buffer

    char* buffer = new char[totalSize];
    char* cursor = buffer;

    //Write to header

    VroomUUID uuid = GetUUID();
    memcpy(cursor, &uuid, sizeof(VroomUUID));
    cursor += sizeof(VroomUUID);
    memcpy(cursor, &texW, sizeof(uint));
    cursor += sizeof(uint);
    memcpy(cursor, &texH, sizeof(uint));
    cursor += sizeof(uint);
    memcpy(cursor, &nChannels, sizeof(uint));
    cursor += sizeof(uint);

    //assetpath:
    memcpy(cursor, &pathLen, sizeof(uint));
    cursor += sizeof(uint); //first we write path length
    memcpy(cursor, assetsPath.c_str(), pathLen); 
    cursor += pathLen; //then we write the actual path

    //maptype
    memcpy(cursor, &typeLen, sizeof(uint));
    cursor += sizeof(uint);
    memcpy(cursor, mapType.c_str(), typeLen);
    cursor += typeLen;

    memcpy(cursor, data, dataSize);

    fs->WriteBinData(libraryPath.c_str(), buffer, totalSize);

    delete[] buffer;

    LOG("Texture saved in: %s", libraryPath.c_str());

}

void ResourceTexture::LoadBin() {
    uint fileSize;
    char* buffer = fs->ReadBinData(libraryPath.c_str(), &fileSize);
    if (!buffer) {
        LOG("Error loading texture: %s", libraryPath.c_str());
        return;
    }

    char* cursor = buffer;

    VroomUUID loadedUUID;
    memcpy(&loadedUUID, cursor, sizeof(VroomUUID));
    cursor += sizeof(VroomUUID);

    memcpy(&texW, cursor, sizeof(uint));
    cursor += sizeof(uint);
    memcpy(&texH, cursor, sizeof(uint));
    cursor += sizeof(uint);
    memcpy(&nChannels, cursor, sizeof(uint));
    cursor += sizeof(uint);

    //Read Assets Path
    uint pathLen;
    memcpy(&pathLen, cursor, sizeof(uint));        
    cursor += sizeof(uint);
    assetsPath.assign(cursor, pathLen);            
    cursor += pathLen;

    //Read Map Type
    uint typeLen;
    memcpy(&typeLen, cursor, sizeof(uint));        
    cursor += sizeof(uint);
    mapType.assign(cursor, typeLen);               
    cursor += typeLen;

    uint dataSize = texW * texH * nChannels;
    data = new unsigned char[dataSize];
    memcpy(data, cursor, dataSize);

    delete[] buffer;

    isLoadedToRAM = true;
    LOG("Texture loaded: %s", libraryPath.c_str());
}

void ResourceTexture::SaveMeta() const {

    std::string metaPath = assetsPath + ".meta";
    nlohmann::json meta = fs->LoadJSON(metaPath.c_str());
    

    meta["uuid"] = GetUUID();
    meta["modTime"] = fs->GetFileModTime(assetsPath);
    meta["name"] = name;
    meta["path"] = assetsPath;
    meta["width"] = texW;
    meta["height"] = texH;
    meta["channels"] = nChannels;
    meta["mapType"] = mapType;

    fs->SaveJSON(metaPath.c_str(), meta);

}



void ResourceTexture::LoadMeta() {
    std::string metaPath = assetsPath + ".meta";

    if (!fs->Exists(metaPath.c_str())) {
        LOG("Meta file not found: %s", metaPath.c_str());
        return;
    }
    nlohmann::json meta = fs->LoadJSON(metaPath.c_str());

    //the uuid is what took us to the meta, so we don't load it
    if (meta.contains("name")) name = meta["name"];
    if (meta.contains("path")) assetsPath = meta["path"];
    if (meta.contains("width")) texW = meta["width"];
    if (meta.contains("height")) texH = meta["height"];
    if (meta.contains("channels")) nChannels = meta["channels"];
    if (meta.contains("mapType")) mapType = meta["mapType"];

}

void ResourceTexture::FreeMemory() {
    if (data) {
        delete[] data;
        data = nullptr;
    }
    isLoadedToRAM = false;
}


void ResourceTexture::LoadToGPU() {

    if (!data) {
        LOG("ERROR: Cannot load texture to GPU - data is NULL!");
        return;
    }

    if (texW == 0 || texH == 0) {
        LOG("ERROR: Cannot load texture to GPU - dimensions are 0!");
        return;
    }

    LOG("Loading texture to GPU: %dx%d, %d channels, data: %p", texW, texH, nChannels, (void*)data);

    if (gpu_id == 0) {
        glGenTextures(1, &gpu_id);
        glBindTexture(GL_TEXTURE_2D, gpu_id);
    }

    GLenum format;
    switch (nChannels) {
    case 1:
        format = GL_RED; break;
    case 3:
        format = GL_RGB; break;
    case 4:
        format = GL_RGBA; break;
    default:
        format = GL_RGB; break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, texW, texH, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //T = Y axis texCoords

    //Filtering mode- -> GL_NEAREST = blocky pattern (default) || GL_LINEAR = smoother pattern
    // can be set separately for minifying or magnifying operations:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG("Texture loaded to GPU: %s (ID: %u)", name.c_str(), gpu_id);

    isLoadedToGPU = true;
}


void ResourceTexture::UnloadFromGPU() {
    if (!isLoadedToRAM) return;
    glDeleteTextures(1, &gpu_id);
    gpu_id = 0;
    isLoadedToGPU = false;
}