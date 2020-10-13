#pragma once

#include "DirectoryBody.h"
#include "SmartEntityName.h"
#include "Drive.h"

class Directory : public DirectoryBody
{
	// None-Static
private:
	Drive* m_Drive;

public:
	SmartEntityName m_Name;

public:
	Directory(unsigned int chankIndex, Drive* drive)
		: m_Drive(drive)
	{
		m_ChankIndex = chankIndex;
		
		// Load Name
		drive->GoToChank(m_ChankIndex);
		m_Name.LoadName(drive->m_FileStream);

		LoadBody();
	}

	void LoadBody();

	unsigned int CreateDirectory(const char name[MAX_ENTITY_NAME + 1]);

	void DeleteMe();

	void DeleteDirectory(unsigned char directoryIndex);
	void DeleteDirectory(const char name[MAX_ENTITY_NAME + 1]);
};