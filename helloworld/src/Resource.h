#pragma once
#include "UUID.h"
#include <string>
#include <vector>
#include <algorithm>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Log.h"



#include "Application.h"
#include "FileSystem.h"


typedef std::uint64_t VroomUUID;
typedef unsigned int uint;

enum ResourceType {
	TEXTURE,
	MODEL,
	MESH,
	MATERIAL,
	AUDIO,
	SCENE,
	BONE,
	ANIM,
	UNKNOWN
};

class Resource {
public:
	Resource(VroomUUID uuid, ResourceType type);
	virtual ~Resource();
	ResourceType GetType() const { return type; }
	VroomUUID GetUUID() const { return uuid; }
	void SetUUID(VroomUUID _uuid) { uuid = _uuid; }

	const char* GetAssetFilePath() const { return assetsPath.c_str(); }
	const char* GetLibraryFilePath() const { return libraryPath.c_str(); }

	void SetAssetFilePath(const std::string& path) { assetsPath = path; }
	void SetLibraryFilePath(const std::string& path) { libraryPath = path; }

	bool IsLoadedToRAM() const { return isLoadedToRAM; }

	uint GetRefCount() const { return refCount; }


	virtual void SaveBin() = 0; //write binary data to Library
	virtual void LoadBin() = 0; //read binary data from Library
	virtual void FreeMemory() = 0; //unload resource from memory, freeing RAM

	virtual void SaveMeta() const = 0;  //.meta to Assets
	virtual void LoadMeta() = 0; //.meta from Assets

protected:
	VroomUUID uuid = 0;
	std::string assetsPath; //Path to original file in Assets/
	std::string libraryPath; //// Path to binary file in Library/
	uint refCount = 0;
	bool isLoadedToRAM = false; // Is data loaded in RAM?

	ResourceType type;

	FileSystem* fs;
};
