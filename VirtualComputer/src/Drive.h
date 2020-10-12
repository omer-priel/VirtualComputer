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
//private:
	std::string m_DrivePath;
	Utils::File m_FileStream;

public:
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
		
		LoadDirectoryBody(m_FileStream);

		// load m_DeletedMemoryList
		{
			m_DeletedMemoryList.ChankIndex = 1;

			do
			{
				m_DeletedMemoryListChanks.push_back(m_DeletedMemoryList.ChankIndex);

				m_FileStream += sizeof(m_DeletedMemoryList.List);
				m_DeletedMemoryList.NextDeletedMemoryList = m_FileStream.Read<unsigned int>();
				if (m_DeletedMemoryList.NextDeletedMemoryList != 0)
				{
					m_DeletedMemoryList.ChankIndex = m_DeletedMemoryList.NextDeletedMemoryList;
					m_FileStream.ChangeIndex(Drive::ChankToFileIndex(m_DeletedMemoryList.ChankIndex));
				}
			} while (m_DeletedMemoryList.NextDeletedMemoryList != 0);

			m_FileStream -= sizeof(m_DeletedMemoryList.List);
			m_FileStream.Read(m_DeletedMemoryList.List);
			
			m_DeletedMemoryList.Index = 0;
			while (m_DeletedMemoryList.List[m_DeletedMemoryList.Index] != 0)
			{
				m_DeletedMemoryList.Index++;
			}
		}
	}

	unsigned int GenerateChank()
	{
		if (m_DeletedMemoryList.Index > 0)
		{
			// Give Deleted Chank
			m_DeletedMemoryList.Index--;
			
			int chankIndex = m_DeletedMemoryList.List[m_DeletedMemoryList.Index];
			
			m_DeletedMemoryList.List[m_DeletedMemoryList.Index] = 0;
			size_t chankIndexInFile = ChankToFileIndex(m_DeletedMemoryList.ChankIndex);
			m_FileStream.Write<unsigned int>(chankIndexInFile + m_DeletedMemoryList.Index, 0);

			return chankIndex;
		}
		else
		{
			if (m_DeletedMemoryList.ChankIndex != 1)
			{
				// Give DeletedMemoryList Chank
				int chankIndex = m_DeletedMemoryList.ChankIndex;
				
				m_DeletedMemoryListChanks.pop_back();

				m_DeletedMemoryList.ChankIndex = m_DeletedMemoryListChanks[m_DeletedMemoryListChanks.size() - 1];
				m_DeletedMemoryList.Index = DELETED_MEMORY_LIST_SIZE;

				m_FileStream.ChangeIndex(ChankToFileIndex(m_DeletedMemoryList.ChankIndex));

				m_FileStream.Read(m_DeletedMemoryList.List);

				m_FileStream.Write<unsigned int>(0);
				m_DeletedMemoryList.NextDeletedMemoryList = 0;
				return chankIndex;
			}
			else
			{
				// Generate new Chank
				int chankIndex = m_ChanksCount;
				m_ChanksCount++;
				return chankIndex;
			}
		}
	}

	void DeleteChank(unsigned int chankIndex)
	{
		if (m_DeletedMemoryList.Index < DELETED_MEMORY_LIST_SIZE)
		{
			m_DeletedMemoryList.List[m_DeletedMemoryList.Index] = chankIndex;
			m_FileStream.Write(ChankToFileIndex(m_DeletedMemoryList.ChankIndex) + m_DeletedMemoryList.Index, chankIndex);
			m_DeletedMemoryList.Index++;
		}
		else
		{
			unsigned int newChankIndex = m_ChanksCount;
			m_ChanksCount++;

			// Update NextDeletedMemoryList
			m_FileStream.Write(ChankToFileIndex(m_DeletedMemoryList.ChankIndex) + DELETED_MEMORY_LIST_SIZE, newChankIndex);

			// In the new Chank
			m_DeletedMemoryListChanks.push_back(newChankIndex);
			
			m_DeletedMemoryList.ChankIndex = newChankIndex;
			m_DeletedMemoryList.List[0] = chankIndex;
			m_DeletedMemoryList.Index = 1;
			std::fill(&m_DeletedMemoryList.List[1], &m_DeletedMemoryList.List[DELETED_MEMORY_LIST_SIZE], 0);
			m_DeletedMemoryList.NextDeletedMemoryList = 0;
			
			m_FileStream.ChangeIndex(ChankToFileIndex(newChankIndex));
			m_FileStream.Write(m_DeletedMemoryList.List);
			m_FileStream.Write(m_DeletedMemoryList.NextDeletedMemoryList);
		}
	}
};

