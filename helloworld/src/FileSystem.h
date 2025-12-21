#pragma once
#include "Module.h"
#include "Log.h"
#include "UUID.h"



#include <fstream>
#include <chrono>
#include <filesystem>

struct FileEntry {
	std::string fullPath;
	std::string name;
	bool isDirectory;
};

class FileSystem : public Module {

public:

	FileSystem();
	~FileSystem();

	bool Start() override;
	bool CleanUp() override;

	bool Exists(const char* path);
	bool CreateDir(const char* path);

	bool CustomCopyFile(const char* src, const char* dest);
	//std::wstring ToWideString(const std::string& utf8Str);


	//binary file handling (for resources)
	char* ReadBinData(const char* filePath, uint* size = nullptr); //load entire file into buffer that is passed as argument (address)
	void WriteBinData(const char* filePath, const char* buffer, uint size); //save entire buffer to file

	void SaveJSON(const char* path, const nlohmann::json& json);
	nlohmann::json LoadJSON(const char* path);

	//Meta file handling 
	void CreateMeta(const char* path, VroomUUID uuid, uint size = 0); //create buffer, fill it & save to Assets


	bool IsMetaValid(const char* metaPath); //check if meta needs to b reimported (modtime, corrupted, missing...)
	bool NeedsReimport(const char* metaPath, const char* sourceFilePath);

	//assets viewer functions
	std::vector<FileEntry> GetDirectoryContents(const char* directory);
	std::vector<std::string> IterateAssetsRecursive(const char* directory);
	bool DeleteFile(const char* path);
	//bool CustomCopyFile(const char* src, const char* dest);
	bool MoveFileToNewPath(const char* oldPath, const char* newPath);


	
	std::string GetFileNameFromPath(const char* path);
	std::string GetDirFromPath(const char* path);
	std::string GetFileFromPath(const char* path);
	std::string GetExtensionFromPath(const char* path);
	VroomUUID GetUUIDFromMeta(const char* metaPath);

	std::string NormalizePath(const char* path);

	uint64_t GetFileModTime(const std::string& path);

	bool ExistsInDirectory(const char* directory, const char* fileName);
	bool ExistsInSubDirectories(const char* directory, const char* fileName);

	bool IsFolderEmpty(const char* path);


};
