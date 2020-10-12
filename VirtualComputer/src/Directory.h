#pragma once

#include "MemoryEntity.h"
#include "DirectoryBody.h"

class Directory : public MemoryEntity
{
	// None-Static
private:
	char m_Name[MAX_ENTITY_NAME + 1];

public:
	DirectoryBody m_Body;

public:
	Directory()
	{
		m_Name[MAX_ENTITY_NAME] = 0; // last byte for print
	}
};

