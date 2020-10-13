#include "pch.h"
#include "SmartEntityName.h"

//Static
bool SmartEntityName::IsEqual(const EntityName& nameA, const EntityName& nameB)
{
	const char* ptrA = &nameA[0];
	const char* ptrB = &nameB[0];
	do
	{
		if (*ptrA != *ptrB)
		{
			return false;
		}
		ptrA++;
		ptrB++;

	} while (*ptrA != 0);

	return true;
}

// None-Static
void SmartEntityName::LoadName(Utils::File& fileStream)
{
	fileStream.Read(&m_Name[0], MAX_ENTITY_NAME);
	m_NameLoaded = true;
}

char* SmartEntityName::GetName()
{
	return &m_Name[0];
}

char* SmartEntityName::GetName(Utils::File& fileStream)
{
	if (!m_NameLoaded)
	{
		LoadName(fileStream);
	}

	return &m_Name[0];
}

void SmartEntityName::Clear()
{
	m_NameLoaded = false;
	m_Name[0] = 0;
}

void SmartEntityName::Change(const EntityName& name)
{
	m_NameLoaded = true;
	
	for (int i = 0; i < MAX_ENTITY_NAME + 1; i++)
	{
		m_Name[i] = name[i];
		if (name[i] == 0)
		{
			break;
		}
	}
}

bool SmartEntityName::IsEqual(const EntityName& name)
{
	return SmartEntityName::IsEqual(m_Name, name);
}