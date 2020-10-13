#include "pch.h"
#include "EntityName.h"


void EntityName::LoadName(Utils::File& fileStream)
{
	if (!m_NameLoaded)
	{
		fileStream.Read(m_Name, MAX_ENTITY_NAME);
		m_NameLoaded = true;
	}
}

char* EntityName::GetName(Utils::File& fileStream)
{
	LoadName(fileStream);

	return m_Name;
}