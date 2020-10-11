#include "pch.h"
#include "Directory.h"

#include <filesystem>

namespace Utils
{
	// Static
	bool Directory::Exists(const char* path, const bool& createIfNeed)
	{
		bool isCreatedNow;
		return Directory::Exists(path, createIfNeed, isCreatedNow);
	}

	bool Directory::Exists(const char* path, const bool& createIfNeed, bool& isCreatedNow)
	{
		std::filesystem::path filesystemPath = path;
		bool exists = std::filesystem::is_directory(filesystemPath);
		if (!exists && createIfNeed)
		{
			isCreatedNow = true;
			exists = std::filesystem::create_directory(filesystemPath);
		}
		else
		{
			isCreatedNow = false;
		}
		return exists;
	}

	inline bool Directory::Exists(const std::string& path, const bool& createIfNeed)
	{
		return Directory::Exists(path.c_str(), createIfNeed);
	}

	inline bool Directory::Exists(const std::string& path, const bool& createIfNeed, bool& isCreatedNow)
	{
		return Directory::Exists(path.c_str(), createIfNeed, isCreatedNow);
	}

	bool Directory::Create(const char* path)
	{
		std::filesystem::path filesystemPath = path;
		return std::filesystem::create_directory(filesystemPath);
	}

	inline bool Directory::Create(const std::string& path)
	{
		return Directory::Create(path.c_str());
	}
}