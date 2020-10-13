#pragma once

#include "MemoryEntity.h"
#include "EntityName.h"

class DirectoryBody : public MemoryEntity
{
public:
	unsigned char m_DirectoriesCount = 0;
	unsigned int m_DirectoriesLocations[MAX_DIRECTORIES];
	EntityName m_DirectoriesNames[MAX_DIRECTORIES];

	unsigned char m_FilesCount = 0;
	unsigned int m_FilesLocations[MAX_DIRECTORIES];
	EntityName m_FilesNames[MAX_DIRECTORIES];
};

