#pragma once

#include <optional>

#include "DirectoryBody.h"
#include "EntityType.h"

namespace VirtualComputer
{
	class Drive : public DirectoryBody
	{
		// Static
	public:
		static unsigned char s_DrivesActives;
		static Drive* s_Drives[MAX_DRIVES];
		static Drive* s_DriveCurrent;

		static void LoadDrives(bool& haveError);

		static Drive* CreateDrive(const char* name = nullptr);
		static char DeleteDrive(const char* name);

		static char DriveNameToIndex(const char* name);

		static constexpr size_t ChankToFileIndex(const unsigned int& chankIndex);

		// None-Static
	public:
		char m_DriveName;

	public:
		Drive(HardDrive* hardDive, char driveName)
			: m_DriveName(driveName)
		{
			m_Drive = hardDive;
			m_ChankIndex = 0;
			LoadBody();
		}

		void GoToThisChank(unsigned int chankIndex, size_t indexInTheChank = 0);

		unsigned int GenerateChank();

		void DeleteChank(unsigned int chankIndex);

		void LoadBody();

		bool ExistName(const EntityName& name);

		unsigned int CreateDirectory(const EntityName& name, const char*& error);

		void DeleteDirectory(unsigned char directoryIndex);

		void RenameDirectory(std::optional<unsigned char>& directoryIndex, unsigned int chankIndex, const EntityName& name, const char*& error);

		unsigned int CreateFile(const EntityName& name, char* content, size_t size, const char*& error);

		void DeleteFile(unsigned char fileIndex);

		void RenameFile(unsigned char fileIndex, const EntityName& name, const char*& error);

		void AddEntity(const EntityType& type, const unsigned int& chankIndex, const EntityName& name);

		void RemoveEntity(const EntityType& type, const unsigned char& entityIndex);

		void RemoveEntity(const EntityType& type, const std::optional<unsigned char> entityIndexOptional, const unsigned int& chankIndex);
	};
}