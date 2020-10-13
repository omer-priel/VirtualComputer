#pragma once

#include "MemoryEntity.h"

class DirectoryBody : public MemoryEntity
{
public:
	unsigned char m_DirectoriesCount = 0;
	unsigned int m_DirectoriesLocations[MAX_DIRECTORIES];
	unsigned char m_FilesCount = 0;
	unsigned int m_FilesLocations[MAX_DIRECTORIES];
};

