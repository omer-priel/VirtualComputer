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

	unsigned int CreateDirectory(const EntityName& name);

	void Delete();

	void DeleteDirectory(unsigned char directoryIndex, const bool& first = true);
	void DeleteDirectory(const EntityName& name);

	void RenameDirectory(unsigned char directoryIndex, const EntityName& name);

	unsigned int CreateFile(const EntityName& name, unsigned int size);

	void DeleteFile(unsigned char directoryIndex, const bool& first = true);
	void DeleteFile(const EntityName& name);

	void RenameFile(unsigned char fileIndex, const EntityName& name);
};