#include "FileSystem.h"


FileSystem::FileSystem() {

}

FileSystem::~FileSystem() {

}

bool FileSystem::Start() {

	return true;
}

bool FileSystem::CleanUp() {

	return true;
}

char* FileSystem::ReadBinData(const char* filePath, uint* size) {
	std::ifstream file(filePath, std::ios::binary);

	if (!file) {  
		LOG("Failed to open %s", filePath);
		return nullptr; 
	}

	// Get file size
	file.seekg(0, std::ios::end);
	size_t len = file.tellg();
	//move back to beginning to start reading
	file.seekg(0, std::ios::beg);  

	
	if (size != nullptr) { 
		*size = (uint)len;
	}

	// Read data
	char* buffer = new char[len];
	file.read(buffer, len);

	// Check if read succeeded
	if (!file) {
		LOG("Failed to read from %s", filePath);
		delete[] buffer;
		file.close();
		return nullptr;
	}

	file.close();
	return buffer;
}

void FileSystem::WriteBinData(const char* filePath, const char* buffer, uint size) {
	std::ofstream file(filePath, std::ios::binary);
	if (file.is_open()) {
		file.write(buffer, size);
		//don't use << operator, since bin data can contain \0 in the middle and truncate your write op
		file.close();
	}
	else {
		LOG("Failed to open file for writing: %s", filePath);
	}
}


void FileSystem::SaveJSON(const char* path, const nlohmann::json& json_to_save) {


	if (!Exists(path)) {

		std::string directory = GetDirFromPath(path);
		/*std::string fileName = GetFileNameFromPath(path);*/
		CreateDir(directory.c_str());

	}

	std::ofstream outputFile(path);
	if (!outputFile) {
		LOG("Failed to open .json file at %s", path);
		return;
	}


	outputFile << std::setw(4) << json_to_save << std::endl;
	outputFile.close(); //would close automatically but doesn't hurt ig


}

void FileSystem::CopyFile(const char* src, const char* dest)
{
	try
	{
		/*std::filesystem::create_directories(std::filesystem::path(dest).parent_path());*/

		std::filesystem::copy_file(src, dest);
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		LOG("Copy failed: %s (%s -> %s)", e.what(), src, dest);
	}
}

std::string FileSystem::GetDirFromPath(const char* path) {

	std::string filePath = NormalizePath(path);
	return filePath.substr(0, filePath.find_last_of('/'));

}

std::string FileSystem::GetFileNameFromPath(const char* path) {

	std::string filePath = NormalizePath(path);

	std::string fileName = filePath.substr(filePath.find_last_of('/') + 1);
	fileName = fileName.substr(0, fileName.find_first_of('.'));
	//remember the file is "[name].fbx.meta", not "[name].fbx"!
	//will return the name, WITHOUT the extension

	return fileName;
}

std::string FileSystem::GetFileFromPath(const char* path) {

	std::string filePath = NormalizePath(path);
	std::string fileName = filePath.substr(filePath.find_last_of('/') + 1);
	//remember the file is "[name].fbx.meta", not "[name].fbx"!
	//will return the name, WITH the extension

	return fileName;
}

std::string FileSystem::GetExtensionFromPath(const char* path) {
	std::string filePath = NormalizePath(path);
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos) return ""; // no extension

	std::string ext = filePath.substr(dotPos + 1);

	//make extension lowercase
	for (char& c : ext) { c = std::tolower(static_cast<unsigned char>(c)); }

	return ext;
}


std::string FileSystem::NormalizePath(const char* path) {
	std::string normalizedPath = path;
	std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
	return normalizedPath;
}

VroomUUID FileSystem::GetUUIDFromMeta(const char* metaPath) {

	if (!IsMetaValid(metaPath)) return 0;

	nlohmann::json meta = LoadJSON(metaPath);
	return meta["uuid"];
}

uint64_t FileSystem::GetFileModTime(const std::string& path) {

	if (!Exists(path.c_str())) {
		LOG("File does not exist: %s", path.c_str());
		return 0;
	}
	try {
		auto lastWriteTime = std::filesystem::last_write_time(path);

		auto fsNow = std::filesystem::file_time_type::clock::now();
		auto sysNow = std::chrono::system_clock::now();

		auto sysClockTimePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastWriteTime - fsNow + sysNow);

		uint64_t unixSeconds = std::chrono::duration_cast<std::chrono::seconds>(sysClockTimePoint.time_since_epoch()).count();

		return unixSeconds;
	}
	catch (const std::exception& e) {
		LOG("Error getting file mod time for %s: %s", path.c_str(), e.what());
		return 0;
	}


}

nlohmann::json FileSystem::LoadJSON(const char* path) { //Whisp's "openFile"!

	std::ifstream inputFile(path);

	if (!inputFile) {
		LOG("Failed to open json file at %s", path);
		return nlohmann::json(); //return empty json file
	}

	nlohmann::json jsonFile;
	inputFile >> jsonFile;
	//inputFile automatically closes when out of scope

	return jsonFile;
}

bool FileSystem::Exists(const char* path) {
	/*bool ret = false;

	std::string filePath = path;
	std::replace(filePath.begin(), filePath.end(), '\\', '/');
	std::string libraryDir = filePath.substr(0, filePath.find_last_of("/"));
	for (const auto& entry : std::filesystem::directory_iterator(libraryDir)) {
		if (entry.is_regular_file()){
			if(entry.path().filename.string() == fileName) { return true; }
		}
	}

	return ret;*/

	return std::filesystem::exists(path); //lol just one line
}

bool FileSystem::CreateDir(const char* path) {

	
	if (Exists(path)) {
		LOG("Directory already exists!");
		return true;
	}
	if (std::filesystem::create_directories(path)) {
		LOG("Directory %s created successfully", path);
		return true;
	}

	LOG("Failed to create directory %s", path);
	return false;

}

void FileSystem::CreateMeta(const char* filePath, const VroomUUID uuid, uint size) {
	nlohmann::json jsonFile;
	std::string metaExt = ".meta";

	std::string metaFilePath = filePath; 
	metaFilePath += metaExt;             
	const char* metaPath = metaFilePath.c_str(); 

	//if (!Exists(metaPath)) CreateDir(metaPath);

	jsonFile["uuid"] = uuid;
	jsonFile["modTime"] = GetFileModTime(filePath);
	jsonFile["fileSize"] = size;


	SaveJSON(metaPath, jsonFile);

}

bool FileSystem::IsMetaValid(const char* metaPath) {
	nlohmann::json meta = LoadJSON(metaPath);

	if (!meta.contains("uuid") || !meta.contains("modTime")) {
		return false;
	}

	return true;
}

bool FileSystem::NeedsReimport(const char* metaPath, const char* sourceFilePath) {
	nlohmann::json meta = LoadJSON(metaPath);
	if (!IsMetaValid(metaPath)) return false;

	uint64_t savedModTime = meta["modTime"]; //checks meta
	uint64_t currentModTime = GetFileModTime(sourceFilePath); //checks source file (fbx)

	return currentModTime != savedModTime;
}

bool FileSystem::ExistsInDirectory(const char* directory, const char* file) {

	std::filesystem::path root = NormalizePath(directory);

	if (std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
		return false;
	}

	std::filesystem::path fullPath = root / file;

	return std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath);
}

bool FileSystem::ExistsInSubDirectories(const char* directory, const char* file) {


	std::string root = NormalizePath(directory);
	if (!Exists(root.c_str()) || !std::filesystem::is_directory(root)) {
		return false;
	}

	for (const auto& entry : std::filesystem::directory_iterator(root)) {
		if (entry.is_regular_file() && entry.path().filename() == file) {
			return true;
		}
	}

	return false;
}

std::vector<std::string> FileSystem::IterateAssetsRecursive(const char* directory) {
	std::vector<std::string> filePaths;
	std::filesystem::path root(directory);

	if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
		LOG("Directory does not exist: %s", directory);
		return filePaths;
	}

	try {
		for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
			if (entry.is_regular_file()) {
				//normalize path before storing
				filePaths.push_back(NormalizePath(entry.path().string().c_str()));
			}
		}
	}
	catch (const std::exception& e) {
		//handle errors
		LOG("Error during recursive directory iteration: %s", e.what());
	}

	return filePaths;
	
}

