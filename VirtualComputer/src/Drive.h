#pragma once

#include "Utils/File.h"

#include "DirectoryBody.h"

class Drive : public DirectoryBody
{
	// Subclass
	struct DeletedMemoryList
	{
		unsigned int ChankIndex = 0;
		
		unsigned int Index;
		unsigned int List[DELETED_MEMORY_LIST_SIZE];

		unsigned int NextDeletedMemoryList; // ChankIndex
	};

	// Static
public:
	static unsigned char s_DrivesActives;
	static Drive* s_Drives[MAX_DRIVES];
	static Drive* s_DriveCurrent;

	static void LoadDrives(bool& haveError);

	static Drive* CreateDrive();

	static constexpr size_t ChankToFileIndex(const unsigned int& chankIndex);

	// None-Static
private:
	std::string m_DrivePath;

public:
	Utils::File m_FileStream;
	char m_DriveName;
	unsigned int m_ChanksCount;

private:
	std::vector<unsigned int> m_DeletedMemoryListChanks;
	DeletedMemoryList m_DeletedMemoryList;

public:
	Drive(const std::string& drivePath, char driveName)
		: m_DrivePath(drivePath), m_FileStream(drivePath), m_DriveName(driveName)
	{
		m_ChankIndex = 0;
		m_ChanksCount = (m_FileStream.Size() + MAX_ENTITY_NAME) / CHANK_SIZE;

		LoadBody();

		// load m_DeletedMemoryList
		m_DeletedMemoryList.ChankIndex = 1;

		do
		{
			m_DeletedMemoryListChanks.push_back(m_DeletedMemoryList.ChankIndex);

			size_t indexOf_NextDeletedMemoryList = Drive::ChankToFileIndex(m_DeletedMemoryList.ChankIndex) + sizeof(m_DeletedMemoryList.List);
			m_FileStream.Read<unsigned int>(indexOf_NextDeletedMemoryList, m_DeletedMemoryList.NextDeletedMemoryList);

			if (m_DeletedMemoryList.NextDeletedMemoryList != 0)
			{
				m_DeletedMemoryList.ChankIndex = m_DeletedMemoryList.NextDeletedMemoryList;
			}
		} while (m_DeletedMemoryList.NextDeletedMemoryList != 0);

		m_FileStream.ChangeIndex(Drive::ChankToFileIndex(m_DeletedMemoryList.ChankIndex));
		m_FileStream.Read(m_DeletedMemoryList.List);

		m_DeletedMemoryList.Index = 0;
		while (m_DeletedMemoryList.List[m_DeletedMemoryList.Index] != 0)
		{
			m_DeletedMemoryList.Index++;
		}
	}

	void GoToChank(unsigned int chankIndex, size_t indexInTheChank = 0);

	unsigned int GenerateChank();

	void DeleteChank(unsigned int chankIndex);

	void LoadBody();

	unsigned int CreateDirectory(const EntityName& name);

	void DeleteDirectory(unsigned char directoryIndex);
	void DeleteDirectory(const EntityName& name);
};

