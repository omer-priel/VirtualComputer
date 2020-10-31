#include "pch.h"
#include "HardDrive.h"

#include "Utils/File.h"
#include "Utils/Directory.h"

namespace VirtualComputer
{
    static const char* DIVERS_PATH = "drvies";
    static const char* DIVER_EXTENSION = ".vhd";

    // Static
    constexpr size_t HardDrive::ChankToFileIndex(const unsigned int& chankIndex)
    {
        if (chankIndex == 0)
            return 0;

        return (CHANK_SIZE * chankIndex) - MAX_ENTITY_NAME;
    }

    // None-Static
    void HardDrive::GoToChank(unsigned int chankIndex, size_t indexInTheChank)
    {
        m_FileStream.ChangeIndex(HardDrive::ChankToFileIndex(chankIndex) + indexInTheChank);
    }

    // Generate and Delete Chanks
    unsigned int HardDrive::GenerateChank()
    {
        size_t originStreamFileIndex = m_FileStream.GetIndex();

        unsigned int chankIndex;
        if (m_DeletedMemoryList.Index > 0)
        {
            // Give Deleted Chank
            m_DeletedMemoryList.Index--;

            chankIndex = m_DeletedMemoryList.List[m_DeletedMemoryList.Index];
            m_DeletedMemoryList.List[m_DeletedMemoryList.Index] = 0;

            GoToChank(m_DeletedMemoryList.ChankIndex, m_DeletedMemoryList.Index * 4);
            m_FileStream.Write<unsigned int>(0);
        }
        else
        {
            if (m_DeletedMemoryList.ChankIndex != 1)
            {
                // Give DeletedMemoryList Chank
                chankIndex = m_DeletedMemoryList.ChankIndex;

                m_DeletedMemoryListChanks.pop_back();

                m_DeletedMemoryList.ChankIndex = m_DeletedMemoryListChanks[m_DeletedMemoryListChanks.size() - 1];
                m_DeletedMemoryList.Index = DELETED_MEMORY_LIST_SIZE;

                GoToChank(m_DeletedMemoryList.ChankIndex);

                m_FileStream.Read(m_DeletedMemoryList.List);

                m_FileStream.Write<unsigned int>(0);
                m_DeletedMemoryList.NextDeletedMemoryList = 0;
            }
            else
            {
                // Generate new Chank
                chankIndex = m_ChanksCount;
                m_ChanksCount++;
            }
        }

        m_FileStream.ChangeIndex(originStreamFileIndex);

        return chankIndex;
    }

    void HardDrive::DeleteChank(unsigned int chankIndex)
    {
        size_t originStreamFileIndex = m_FileStream.GetIndex();

        if (m_DeletedMemoryList.Index < DELETED_MEMORY_LIST_SIZE)
        {
            m_DeletedMemoryList.List[m_DeletedMemoryList.Index] = chankIndex;
            GoToChank(m_DeletedMemoryList.ChankIndex, m_DeletedMemoryList.Index * 4);
            m_FileStream.Write(chankIndex);
            m_DeletedMemoryList.Index++;
        }
        else
        {
            unsigned int newChankIndex = m_ChanksCount;
            m_ChanksCount++;

            // Update NextDeletedMemoryList
            GoToChank(m_DeletedMemoryList.ChankIndex, m_DeletedMemoryList.Index * 4);
            m_FileStream.Write(newChankIndex);

            // In the new Chank
            m_DeletedMemoryListChanks.push_back(newChankIndex);

            m_DeletedMemoryList.ChankIndex = newChankIndex;
            m_DeletedMemoryList.List[0] = chankIndex;
            m_DeletedMemoryList.Index = 1;
            std::fill(&m_DeletedMemoryList.List[1], &m_DeletedMemoryList.List[DELETED_MEMORY_LIST_SIZE], 0);
            m_DeletedMemoryList.NextDeletedMemoryList = 0;

            GoToChank(newChankIndex);
            m_FileStream.Write(m_DeletedMemoryList.List);
            m_FileStream.Write(m_DeletedMemoryList.NextDeletedMemoryList);
        }

        m_FileStream.ChangeIndex(originStreamFileIndex);

        Logger::Info("Chank ", chankIndex, " deleted.");
    }
}