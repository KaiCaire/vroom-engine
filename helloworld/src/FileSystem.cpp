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

	// this completely clears the file before writing, ensuring no corrupted data remains from previous attempts.
	std::ofstream outputFile(std::filesystem::u8path(path), std::ios::out | std::ios::trunc);
	if (!outputFile) {
		LOG("Failed to open .json file at %s", path);
		return;
	}


	outputFile << std::setw(4) << json_to_save << std::endl;
	outputFile.close(); //would close automatically but doesn't hurt ig


}

bool FileSystem::CustomCopyFile(const char* src, const char* dest)
{
	//must convert strings to system::path objects, otherwise it won't detect accented characters properly!
	try {
		std::filesystem::path srcPath = std::filesystem::u8path(src);
		std::filesystem::path dstPath = std::filesystem::u8path(dest);

		// Ensure directory exists
		if (dstPath.has_parent_path()) {
			std::filesystem::create_directories(dstPath.parent_path());
		}

		//check source file exists (just in case ig)
		if (!std::filesystem::exists(srcPath)) {
			// This LOG will likely still show garbage because the console 
			// doesn't support UTF-8, but the check should now PASS.
			LOG("ERROR: Source file not found even with UTF8 parsing.");
			return false;
		}

		return std::filesystem::copy_file(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing);
	}
	catch (const std::filesystem::filesystem_error& e) {
		LOG("Copy failed: %s", e.what());
		return false;
	}
}

////helper to properly detect accented characters and all that stuff
//std::wstring FileSystem::ToWideString(const std::string& utf8Str) {
//	if (utf8Str.empty()) return L"";
//	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), NULL, 0);
//	std::wstring wstrTo(size_needed, 0);
//	MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), &wstrTo[0], size_needed);
//	return wstrTo;
//}

std::string FileSystem::GetDirFromPath(const char* path) {

	std::string filePath = NormalizePath(path);
	return filePath.substr(0, filePath.find_last_of('/'));

}

std::string FileSystem::GetFileNameFromPath(const char* path) {
	std::string filePath = NormalizePath(path);

	// 1. Get the part after the last slash (e.g., "BakerHouse.fbx")
	size_t lastSlash = filePath.find_last_of('/');
	std::string fileName = (lastSlash == std::string::npos) ? filePath : filePath.substr(lastSlash + 1);

	// 2. Find the LAST dot to strip the extension (e.g., "BakerHouse.fbx" -> "BakerHouse")
	size_t lastDot = fileName.find_last_of('.');
	if (lastDot != std::string::npos) {
		fileName = fileName.substr(0, lastDot);
	}

	//Special case for meta files: "modelName.fbx.meta"
	size_t metaCheck = fileName.find_last_of('.');
	if (metaCheck != std::string::npos) {
		fileName = fileName.substr(0, metaCheck);
	}

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
	if (path == nullptr || strlen(path) == 0) return "";

	// 1. Create a filesystem path object (UTF-8 safe)
	std::filesystem::path p = std::filesystem::u8path(path);

	// 2. Use lexically_normal() to resolve ".." and "." without checking if the file exists
	// 3. Use generic_string() to ensure forward slashes '/' on all platforms
	std::string normalized = p.lexically_normal().generic_string();

	return normalized;
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

nlohmann::json FileSystem::LoadJSON(const char* path) {
	std::ifstream inputFile(path);

	if (!inputFile) {
		LOG("Failed to open json file at %s", path);
		return nlohmann::json();  // Return empty JSON
	}

	try {
		nlohmann::json jsonFile;
		inputFile >> jsonFile;
		return jsonFile;
	}
	catch (const nlohmann::json::parse_error& e) {
		LOG("ERROR: JSON parse error in %s: %s", path, e.what());
		return nlohmann::json();  // Return empty JSON on error
	}
}

bool FileSystem::Exists(const char* path) {
	if (path == nullptr || strlen(path) == 0) return false;

	try {
		// We create a path object from the UTF-8 string handle accented characters (à, ó, etc.)
		std::filesystem::path p = std::filesystem::u8path(path);
		return std::filesystem::exists(p);
	}
	catch (const std::exception& e) {
		// Just in case of weird permission issues or illegal characters
		return false;
	}
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
	const char* metaDir = (filePath + metaExt).c_str();

	// Make sure directory exists
	std::string dir = GetDirFromPath(metaDir);
	if (!Exists(dir.c_str())) {
		CreateDir(dir.c_str());
	}

	jsonFile["uuid"] = uuid;
	jsonFile["modTime"] = GetFileModTime(filePath);

	SaveJSON(metaDir, jsonFile);  
}

bool FileSystem::IsMetaValid(const char* metaPath) {
	// Read the first few characters of the file as a raw string just to check if it's valid
	std::ifstream file(metaPath);
	if (!file.is_open()) return false;

	std::string line;
	std::getline(file, line);
	file.close();

	// Unity metas usually start with "fileFormatVersion:" 
	// Vroom metas (JSON) must start with '{'
	if (line.find("{") == std::string::npos) {
		LOG("Ignoring Unity meta file: %s", metaPath);
		return false;
	}

	nlohmann::json meta = LoadJSON(metaPath);
	if (meta.is_null() || !meta.contains("uuid") || !meta.contains("modTime")) {
		return false;
	}

	return true;
}
bool FileSystem::NeedsReimport(const char* metaPath, const char* sourceFilePath) {
	nlohmann::json meta = LoadJSON(metaPath);
	if (!IsMetaValid(metaPath)) return false;

	uint64_t savedModTime = meta["modTime"]; //checks meta
	uint64_t currentModTime = GetFileModTime(sourceFilePath); //checks source file (fbx)

	if (currentModTime != savedModTime) return true;
	else return false;
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

bool FileSystem::DeleteFile(const char* filePath) {
	//check if file path exists
	std::string path = NormalizePath(filePath);
	if (!Exists(path.c_str())) {
		LOG("WARNING: Cannot delete file %s - does not exist.", path);
		return true; 
	}

	std::error_code ec;
	if (std::filesystem::remove(path, ec)) {
		LOG("Deleted file: %s", path);
		return true;
	}
	LOG("ERROR: Failed to delete file %s. Reason: %s", path, ec.message().c_str());
	return false;
}

//bool FileSystem::CopyFile(const char* src, const char* dest) {
//	if (!Exists(src)) {
//		LOG("ERROR: Source file for copy does not exist: %s", src);
//		return false;
//	}
//
//	std::error_code ec;
//	//copy file
//	std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing, ec);
//
//	if (ec) {
//		LOG("ERROR: Failed to copy file from %s to %s. Reason: %s", src, dest, ec.message().c_str());
//		return false;
//	}
//	LOG("Successfully copied file to %s", dest);
//	return true;
//}

std::vector<FileEntry> FileSystem::GetDirectoryContents(const char* directory) {
	std::vector<FileEntry> entries;
	std::filesystem::path root(directory);

	if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
		return entries;
	}

	try {
		for (const auto& entry : std::filesystem::directory_iterator(root)) {
			FileEntry fe;
			fe.fullPath = NormalizePath(entry.path().string().c_str());
			fe.name = entry.path().filename().string();
			fe.isDirectory = entry.is_directory();

			//skip hidden files
			if (fe.name.front() == '.') continue;

			//skip meta files 
			if (fe.name.find(".meta") != std::string::npos) continue;

			//skip other unwanted files
			if (!fe.isDirectory) {
				std::string nameLower = fe.name;
				std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

				if (nameLower.find(".js") != std::string::npos ||
					nameLower.find(".html") != std::string::npos ||
					nameLower.find(".cmake") != std::string::npos ||
					nameLower.find(".pdb") != std::string::npos ||
					nameLower.find(".vcxproj") != std::string::npos) {
					continue;
				}
			}

			entries.push_back(fe);
		}
	}
	catch (const std::exception& e) {
		LOG("Error during directory iteration: %s", e.what());
	}

	std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
		if (a.isDirectory != b.isDirectory) {
			return a.isDirectory > b.isDirectory;
		}
		return a.name < b.name;
		});

	return entries;
}

bool FileSystem::MoveFileToNewPath(const char* oldPath, const char* newPath) {
	//check if path exists
	if (!Exists(oldPath)) {
		LOG("ERROR: Cannot move file - source does not exist: %s", oldPath);
		return false;
	}

	std::error_code ec;
	//move file
	std::filesystem::rename(oldPath, newPath, ec);

	if (ec) {
		LOG("ERROR: Failed to move/rename %s to %s. Reason: %s", oldPath, newPath, ec.message().c_str());
		return false;
	}

	LOG("Successfully moved file from %s to %s", oldPath, newPath);
	return true;
}


bool FileSystem::IsFolderEmpty(const char* path) {

	std::string folderPath = NormalizePath(path);

	if (!std::filesystem::exists(path)) {
		LOG("ERROR: Path does not exist: %s", path);
		return true;
	}

	if (!std::filesystem::is_directory(path)) {
		LOG("ERROR: Path is not a directory: %s", path);
		return true;
	}

	return std::filesystem::is_empty(folderPath);

}

