#pragma once

#include "Utils/File.h"

class SmartEntityName
{
	//Static
public:
	static bool IsEqual(const EntityName& nameA, const EntityName& nameB);

	//None-Static
public:
	bool m_NameLoaded;
	EntityName m_Name;

	SmartEntityName()
	{
		m_NameLoaded = false;
		m_Name[MAX_ENTITY_NAME] = 0; // last byte for print
	}

	SmartEntityName(const EntityName& name)
	{
		m_NameLoaded = true;
		m_Name = name;
	}

	void LoadName(Utils::File& fileStream);
	
	char* GetName();
	char* GetName(Utils::File& fileStream);
	
	void Clear();
	void Change(const EntityName& name);

	bool IsEqual(const EntityName& name);
};
