#pragma once

#include <optional>

#include "MemoryEntity.h"
#include "HardDrive.h"
#include "SmartEntityName.h"
#include "EntityType.h"

namespace VirtualComputer
{
	class DirectoryBody : public MemoryEntity
	{
		// Static
	public:
		static bool CheakEntityName(const EntityName& name);

		// None-Static
	public:
		HardDrive* m_Drive;

		// Body
		unsigned char m_DirectoriesCount = 0;
		unsigned int m_DirectoriesLocations[MAX_DIRECTORIES];
		SmartEntityName m_DirectoriesNames[MAX_DIRECTORIES];

		unsigned char m_FilesCount = 0;
		unsigned int m_FilesLocations[MAX_DIRECTORIES];
		SmartEntityName m_FilesNames[MAX_DIRECTORIES];

		DirectoryBody()
		{

		}

		DirectoryBody(const unsigned int& chankIndex, HardDrive* drive)
			: m_Drive(drive)
		{
			m_ChankIndex = chankIndex;
		}

		// Virtual functions
		virtual void GoToThisChankBody(size_t indexInTheChank = 0);

		// Load
		void LoadBody();

		// Info
		bool ExistName(const EntityName& name);

		// Entity Actions
		void AddEntity(const EntityType& type, const unsigned int& chankIndex, const EntityName& name);

		void RemoveEntity(const EntityType& type, const unsigned char& entityIndex);

		void RemoveEntity(const EntityType& type, const std::optional<unsigned char> entityIndexOptional, const unsigned int& chankIndex);

		// Directories Actions
		unsigned int CreateDirectory(const EntityName& name, const char*& error);

		void DeleteDirectory(unsigned char directoryIndex, const bool& first = true);

		void RenameDirectory(std::optional<unsigned char>& directoryIndex, unsigned int chankIndex, const EntityName& name, const char*& error);

		// Files Actions
		unsigned int CreateFile(const EntityName& name, char* content, size_t size, const char*& error);

		void DeleteFile(unsigned char fileIndex, const bool& first = true);

		void RenameFile(unsigned char fileIndex, const EntityName& name, const char*& error);
	};
}