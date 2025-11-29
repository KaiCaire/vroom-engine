#include "Importer.h"
#include "Model.h"
#include "MeshImporter.h"
#include "TextureImporter.h"
// #include "MaterialImporter.h"
#include "Application.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

Importer::Importer() {
  
}

Importer::~Importer() {}

bool Importer::Start() {

    // Stream log messages to Log window
    struct aiLogStream stream;
    stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
    aiAttachLogStream(&stream);

    //sceneImporter = new SceneImporter();
    meshImporter = new MeshImporter();
    textureImporter = new TextureImporter();
    /* materialImporter = new MaterialImporter();*/
    return true;
}

bool Importer::CleanUp() {
  /*  delete sceneImporter;*/
    delete meshImporter;
    delete textureImporter;
    //delete materialImporter;

    /*sceneImporter = nullptr;*/
    meshImporter = nullptr;
    textureImporter = nullptr;
    //materialImporter = nullptr;
    return true;
}