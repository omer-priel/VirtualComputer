#pragma once

#include "MemoryEntity.h"
#include "SmartEntityName.h"

namespace VirtualComputer
{
	class DirectoryBody : public MemoryEntity
	{
	public:
		unsigned char m_DirectoriesCount = 0;
		unsigned int m_DirectoriesLocations[MAX_DIRECTORIES];
		SmartEntityName m_DirectoriesNames[MAX_DIRECTORIES];

		unsigned char m_FilesCount = 0;
		unsigned int m_FilesLocations[MAX_DIRECTORIES];
		SmartEntityName m_FilesNames[MAX_DIRECTORIES];
	};
}