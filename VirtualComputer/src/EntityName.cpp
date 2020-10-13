#include "pch.h"
#include "EntityName.h"

//Static
bool EntityName::IsEqual(char nameA[MAX_ENTITY_NAME + 1], char nameB[MAX_ENTITY_NAME + 1])
{
	char* ptrA = &nameA[0];
	char* ptrB = &nameB[0];
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
void EntityName::LoadName(Utils::File& fileStream)
{
	fileStream.Read(m_Name, MAX_ENTITY_NAME);
	m_NameLoaded = true;
}

char* EntityName::GetName(Utils::File& fileStream)
{
	if (!m_NameLoaded)
	{
		LoadName(fileStream);
	}

	return m_Name;
}

void EntityName::Clear()
{
	m_NameLoaded = false;
	m_Name[0] = 0;
}

void EntityName::Change(char name[MAX_ENTITY_NAME + 1])
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

bool EntityName::IsEqual(char name[MAX_ENTITY_NAME + 1])
{
	return EntityName::IsEqual(m_Name, name);
}