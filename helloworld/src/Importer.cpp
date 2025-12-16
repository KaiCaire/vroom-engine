#include "Importer.h"
#include "ModelImporter.h"
#include "MeshImporter.h"
#include "TextureImporter.h"
// #include "MaterialImporter.h"
#include "Application.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

Importer::Importer() {
    modelImporter = new ModelImporter();
    meshImporter = new MeshImporter();
    textureImporter = new TextureImporter();
}

Importer::~Importer() {}

bool Importer::Start() {

    // Stream log messages to Log window
    //struct aiLogStream stream;
    //stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
    //aiAttachLogStream(&stream);

    
    
    /* materialImporter = new MaterialImporter();*/
    return true;
}

bool Importer::CleanUp() {
    delete modelImporter;
    delete meshImporter;
    delete textureImporter;
    //delete materialImporter;

    modelImporter = nullptr;
    meshImporter = nullptr;
    textureImporter = nullptr;
    //materialImporter = nullptr;
    return true;
}