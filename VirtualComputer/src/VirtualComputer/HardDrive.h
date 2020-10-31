#pragma once

namespace VirtualComputer
{
	class HardDrive
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
		static constexpr size_t ChankToFileIndex(const unsigned int& chankIndex);

		// None-Static
	private:
		std::string m_DrivePath;

	public:
		Utils::File m_FileStream;
		unsigned int m_ChanksCount;

	private:
		std::vector<unsigned int> m_DeletedMemoryListChanks;
		DeletedMemoryList m_DeletedMemoryList;

	public:
		HardDrive(const std::string& drivePath)
			: m_DrivePath(drivePath), m_FileStream(drivePath)
		{
			m_ChanksCount = (m_FileStream.Size() + MAX_ENTITY_NAME) / CHANK_SIZE;

			// load m_DeletedMemoryList
			m_DeletedMemoryList.ChankIndex = 1;

			do
			{
				m_DeletedMemoryListChanks.push_back(m_DeletedMemoryList.ChankIndex);

				size_t indexOf_NextDeletedMemoryList = HardDrive::ChankToFileIndex(m_DeletedMemoryList.ChankIndex) + sizeof(m_DeletedMemoryList.List);
				m_FileStream.Read<unsigned int>(indexOf_NextDeletedMemoryList, m_DeletedMemoryList.NextDeletedMemoryList);

				if (m_DeletedMemoryList.NextDeletedMemoryList != 0)
				{
					m_DeletedMemoryList.ChankIndex = m_DeletedMemoryList.NextDeletedMemoryList;
				}
			} while (m_DeletedMemoryList.NextDeletedMemoryList != 0);

			m_FileStream.ChangeIndex(HardDrive::ChankToFileIndex(m_DeletedMemoryList.ChankIndex));
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
	};
}