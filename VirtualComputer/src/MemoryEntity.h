#pragma once

#include "Utils/File.h"

class MemoryEntity
{
public:
	unsigned int m_ChankIndex;

	bool m_NameLoaded = false;
	char m_Name[MAX_ENTITY_NAME + 1];

	MemoryEntity()
	{
		m_Name[MAX_ENTITY_NAME] = 0; // last byte for print
	}

	void LoadName(Utils::File& fileStream);
	char* GetName(Utils::File& fileStream);
};

