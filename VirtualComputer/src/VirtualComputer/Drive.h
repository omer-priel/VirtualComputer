#pragma once

#include <optional>

#include "DirectoryBody.h"
#include "EntityType.h"

namespace VirtualComputer
{
	class Drive : public DirectoryBody
	{
		// Static
	public:
		static unsigned char s_DrivesActives;
		static Drive* s_Drives[MAX_DRIVES];
		static Drive* s_DriveCurrent;

		static void LoadDrives(bool& haveError);

		static Drive* CreateDrive(const char* name = nullptr);
		static char DeleteDrive(const char* name);

		static char DriveNameToIndex(const char* name);

		static constexpr size_t ChankToFileIndex(const unsigned int& chankIndex);

		// None-Static
	public:
		char m_DriveName;

	public:
		Drive(HardDrive* hardDrive, char driveName)
			: DirectoryBody(0, hardDrive), m_DriveName(driveName)
		{
			LoadBody();
		}

		~Drive()
		{
			if (m_Drive != nullptr)
			{
				if (m_Drive->m_FileStream.IsOpened())
				{
					m_Drive->m_FileStream.Close();
				}
				delete m_Drive;
			}
		}

		void GoToThisChankBody(size_t indexInTheChank = 0) override;
	};
}