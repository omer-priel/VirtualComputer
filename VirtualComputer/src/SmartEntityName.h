#pragma once

#include "Utils/File.h"

class SmartEntityName
{
	//Static
public:
	static bool IsEqual(char nameA[MAX_ENTITY_NAME + 1], char nameB[MAX_ENTITY_NAME + 1]);

	//None-Static
public:
	bool m_NameLoaded;
	char m_Name[MAX_ENTITY_NAME + 1];

	SmartEntityName()
	{
		m_NameLoaded = false;
		m_Name[MAX_ENTITY_NAME] = 0; // last byte for print
	}

	void LoadName(Utils::File& fileStream);
	char* GetName(Utils::File& fileStream);
	
	void Clear();
	void Change(char name[MAX_ENTITY_NAME + 1]);

	bool IsEqual(char name[MAX_ENTITY_NAME + 1]);
};