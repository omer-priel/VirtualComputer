#pragma once

#include "Utils/File.h"

#include "DirectoryBody.h"

class Drive
{
	// Static
public:
	static unsigned char s_DrivesActives;
	static Drive* s_Drives[MAX_DRIVES];
	static Drive* s_DriveCurrent;

	static void LoadDrives(bool& haveError);

	static Drive* CreateDrive();

	// None-Static
//private:
	std::string m_DrivePath;
	Utils::File m_FileStream;

	char m_DriveName;

public:
	DirectoryBody m_Body;

public:
	Drive(const std::string& drivePath, char driveName)
		: m_DrivePath(drivePath), m_FileStream(drivePath), m_DriveName(driveName)
	{
		m_Body.LoadBody(m_FileStream);
	}
};

