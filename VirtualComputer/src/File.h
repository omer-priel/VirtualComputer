#pragma once

#include "MemoryEntity.h"
#include "SmartEntityName.h"
#include "Drive.h"

class File : public MemoryEntity
{
	// Subclasses
	struct FileBodyChank
	{
		unsigned int ChankIndex = 0;
		char Body[CHANK_SIZE - 4];
	};

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

	void LoadBody();
	void DeleteBody(Drive* drive);
};

