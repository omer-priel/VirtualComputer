#pragma once

#include "Utils/File.h"

namespace VirtualComputer
{
	class SmartEntityName
	{
		//Static
	public:
		static bool IsEqual(const EntityName& nameA, const EntityName& nameB);
		static bool IsEqual(const char* nameA, const char* nameB, const size_t sizeA, const size_t sizeB);

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
}