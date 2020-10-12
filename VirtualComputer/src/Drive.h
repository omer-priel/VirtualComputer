#pragma once

#include "Utils/File.h"

#include "DirectoryBody.h"

class Drive : public DirectoryBody
{
	// Static
public:
	static unsigned char s_DrivesActives;
	static Drive* s_Drives[MAX_DRIVES];
	static Drive* s_DriveCurrent;

	static void LoadDrives(bool& haveError);

	static Drive* CreateDrive();

	size_t ChankToFileIndex(const unsigned int& chankIndex);

	// None-Static
//private:
	std::string m_DrivePath;
	Utils::File m_FileStream;

	char m_DriveName;
	unsigned m_ChanksCount;

public:
	Drive(const std::string& drivePath, char driveName)
		: m_DrivePath(drivePath), m_FileStream(drivePath), m_DriveName(driveName)
	{
		m_Location = 0;
		m_ChanksCount = (m_FileStream.Size() + MAX_ENTITY_NAME) / CHANK_SIZE;
		
		LoadDirectoryBody(m_FileStream);
	}
};

