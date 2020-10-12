#pragma once

#include "Utils/File.h"

class EntityName
{
public:
	bool m_NameLoaded = false;
	char m_Name[MAX_ENTITY_NAME + 1];

	EntityName()
	{
		m_Name[MAX_ENTITY_NAME] = 0; // last byte for print
	}

	void LoadName(Utils::File& fileStream);
	char* GetName(Utils::File& fileStream);
};
