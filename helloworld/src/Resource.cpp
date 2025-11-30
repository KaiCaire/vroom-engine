#include "Resource.h"
#include "Application.h"

Resource::Resource(ResourceType _type) : uuid(0), type(_type), isLoadedToRAM(false), referenceCount(0), fs(nullptr) // Initialize the pointer before assignment
{
    fs = Application::GetInstance().fileSystem.get();
}

