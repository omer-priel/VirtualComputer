#pragma once

#include "MemoryEntity.h"
#include "SmartEntityName.h"
#include "Drive.h"

namespace VirtualComputer
{
	class File : public MemoryEntity
	{
		// Subclasses
		struct FileBodyChank
		{
			unsigned int ChankIndex = 0;
			char Body[CHANK_SIZE - 4];
		};

		// Static
	public:
		static unsigned int Create(Drive* drive, const EntityName& name, char* content, size_t size);
		static void DeleteFile(Drive* drive, unsigned int chankIndex);

		// None-Static
	private:
		Drive* m_Drive;

	public:
		SmartEntityName m_Name;
		unsigned int m_Size;

		char m_FirstBodyChank[FIRST_FILE_BODY_SIZE];
		std::vector<FileBodyChank*> m_BodyChanks;

		File(unsigned int chankIndex, Drive* drive)
			: m_Drive(drive)
		{
			m_ChankIndex = chankIndex;

			// Load Name
			drive->GoToChank(m_ChankIndex);
			m_Name.LoadName(drive->m_FileStream);

			LoadBody();
		}

		~File()
		{
			for (auto ptr : m_BodyChanks)
			{
				delete ptr;
			}
		}

		void LoadBody();
	};
}