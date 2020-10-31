#pragma once

#include "DirectoryBody.h"
#include "SmartEntityName.h"
#include "HardDrive.h"

namespace VirtualComputer
{
	class Directory : public DirectoryBody
	{
		// None-Static
	public:
		SmartEntityName m_Name;

	public:
		Directory(const unsigned int& chankIndex, HardDrive* drive)
			: DirectoryBody(chankIndex, drive)
		{
			// Load Name
			drive->GoToChank(m_ChankIndex);
			m_Name.LoadName(drive->m_FileStream);

			LoadBody();
		}

		void GoToThisChankBody(size_t indexInTheChank = 0) override;
	};
}