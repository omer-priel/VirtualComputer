#pragma once

#include "MemoryEntity.h"
#include "SmartEntityName.h"

#include "HardDrive.h"

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

		// Abstract functions
		virtual void GoToThisChank(unsigned int chankIndex, size_t indexInTheChank) = 0;
	};
}