#pragma once

namespace Utils
{
	class Directory
	{
		// Static
	public:
		static bool Exists(const char* path, const bool& createIfNeed = true);

		static bool Exists(const char* path, const bool& createIfNeed, bool& isCreatedNow);

		static inline bool Exists(const std::string& path, const bool& createIfNeed = true);

		static inline bool Exists(const std::string& path, const bool& createIfNeed, bool& isCreatedNow);

		static bool Create(const char* path);

		static inline bool Create(const std::string& path);
	};
}