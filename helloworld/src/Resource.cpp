#include "Resource.h"
#include "Application.h"

Resource::Resource(ResourceType _type) : uuid(0), type(_type), isLoadedToRAM(false), referenceCount(0), fs(nullptr) // Initialize the pointer before assignment
{
    fs = Application::GetInstance().fileSystem.get();
}

std::string Resource::GetTypeString(ResourceType type) {
    switch (type) {
    case ResourceType::MESH:
        return "Mesh";
    case ResourceType::SCENE:
        return "Scene";
    case ResourceType::TEXTURE:
        return "Texture";
    case ResourceType::MATERIAL:
        return "Material";
    case ResourceType::SHADER:
        return "Shader";
    case ResourceType::AUDIO:
        return "Audio";
    case ResourceType::UNKNOWN:
    default:
        return "Unknown";
    }
}

