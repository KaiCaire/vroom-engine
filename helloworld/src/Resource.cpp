#include "Resource.h"
#include "Application.h"


namespace Paths {
    const char* ASSETS_DIR = "../Assets";
    const char* MODEL_ASSETS_DIR = "../Assets/Models";
    const char* TEXTURE_ASSETS_DIR = "../Assets/Textures";
    const char* SCENE_ASSETS_DIR = "../Assets/Scenes";
    const char* MESH_LIB_DIR = "Library/Meshes/";
    const char* TEXTURE_LIB_DIR = "Library/Textures/";
    const char* MODEL_LIB_DIR = "Library/Models/";
    const char* MATERIAL_LIB_DIR = "Library/Materials/";
}

Resource::Resource(VroomUUID _uuid, ResourceType _type) {
	uuid = _uuid;
	type = _type;
	fs = Application::GetInstance().fileSystem.get();
}

Resource::~Resource() {

}





