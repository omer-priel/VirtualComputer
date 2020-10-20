#include "pch.h"
#include "Drive.h"

#include "Utils/Directory.h"

#include "DirectoryBody.h"
#include "Directory.h"
#include "File.h"

namespace VirtualComputer
{
    static const char* DIVERS_PATH = "drvies";
    static const char* DIVER_EXTENSION = ".vhd";

    // Static
    unsigned char Drive::s_DrivesActives = 0;
    Drive* Drive::s_Drives[MAX_DRIVES];
    Drive* Drive::s_DriveCurrent = nullptr;

    void Drive::LoadDrives(bool& haveError)
    {
        bool drivesDirectoryExist = Utils::Directory::Exists(DIVERS_PATH, true);
        if (!drivesDirectoryExist)
        {
            std::cout << "Can't create drvies directory!\n";
            haveError = true;
            return;
        }

        std::string drivePath = DIVERS_PATH;
        drivePath += "//drive01";
        drivePath += DIVER_EXTENSION;
        
        size_t numberIndex = strlen(DIVERS_PATH) + 7;

        char driveName = 'A';
        for (int i = 0; i < MAX_DRIVES; i++)
        {
            drivePath[numberIndex] = (char)('0' + ((i + 1) / 10));
            drivePath[numberIndex + 1] = (char)('0' + ((i + 1) % 10));

            if (Utils::File::Exists(drivePath.c_str()))
            {
                s_Drives[i] = new Drive(drivePath, driveName);
                s_DrivesActives++;

                if (Drive::s_DriveCurrent == nullptr)
                {
                    Drive::s_DriveCurrent = s_Drives[i];
                }
            }

            driveName++;
        }

        if (s_DrivesActives == 0) // Not exists drives
        {
            CreateDrive();
            Drive::s_DriveCurrent = s_Drives[0];
        }
    }

    Drive* Drive::CreateDrive()
    {
        if (s_DrivesActives == MAX_DRIVES)
        {
            std::cout << "Can't create more the " << MAX_DRIVES << " drives.\n";
            return nullptr;
        }

        int index = 0;
        while (s_Drives[index] != nullptr)
        {
            index++;
        }

        std::string drivePath = DIVERS_PATH;
        drivePath += "//drive";
        drivePath += (char)('0' + ((index + 1) / 10));
        drivePath += (char)('0' + ((index + 1) % 10));
        drivePath += DIVER_EXTENSION;

        if (Utils::File::Exists(drivePath.c_str()))
        {
            Utils::File::Resize(drivePath, 1);
        }

        Utils::File fileStream(drivePath, true);

        std::array<char, (CHANK_SIZE - MAX_ENTITY_NAME) + CHANK_SIZE> data; // CHANK_SIZE - MAX_ENTITY_NAME = Drive, CHANK_SIZE = DeletedMemoryList
        data.fill(0);
        fileStream.Write(&data[0], data.size());
        fileStream.Close();

        char driveName = (char)('A' + index);

        Drive* drive = new Drive(drivePath, driveName);

        s_Drives[index] = drive;
        s_DrivesActives++;

        return drive;
    }

    char Drive::DeleteDrive(const char* name)
    {
        char index = DriveNameToIndex(name);
        
        if (index == -1)
        {
            std::cout << "Drive name syntex error!\n";
            return 0;
        }

        Drive* drive = Drive::s_Drives[index];

        if (drive == nullptr)
        {
            std::cout << "the drive not exists!\n";
            return 0;
        }

        if (drive == Drive::s_DriveCurrent)
        {
            std::cout << "Can't delete the current drive!\n";
            return 0;
        }

        // need code - need Utils::File::Delete function

        return ('A' + index);
    }

    char Drive::DriveNameToIndex(const char* name)
    {
        size_t size = strlen(name);
        
        if (size == 1 || (size == 2 && name[1] == ':'))
        {
            if ('a' <= name[0] && name[0] <= 'z')
            {
                return name[0] - 'a';
            }

            if ('A' <= name[0] && name[0] <= 'Z')
            {
                return name[0] - 'A';
            }
        }

        return -1;
    }

    constexpr size_t Drive::ChankToFileIndex(const unsigned int& chankIndex)
    {
        if (chankIndex == 0)
            return 0;

        return (CHANK_SIZE * chankIndex) - MAX_ENTITY_NAME;
    }

    bool Drive::CheakEntityName(const EntityName& name)
    {
        if (name.empty())
        {
            Logger::Error("The entity name can't be empty!"); // fix: entity need be directory or file.
            return false;
        }

        for (char tv : name)
        {
            if (tv == 0)
            {
                break;
            }
            if (tv == '.' || tv == ':' || tv == '\\' || tv == '\'' || tv == '\"' || tv == '\n')
            {
                Logger::Error("The tv \'", tv, "\' can't be in entity name!"); // fix: entity need be directory or file.
                return false;
            }
        }

        return true;
    }

    // None-Static
    void Drive::GoToChank(unsigned int chankIndex, size_t indexInTheChank)
    {
        m_FileStream.ChangeIndex(Drive::ChankToFileIndex(chankIndex) + indexInTheChank);
    }

    // Generate and Delete Chanks
    unsigned int Drive::GenerateChank()
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

    void Drive::DeleteChank(unsigned int chankIndex)
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

    // Body Actions
    void Drive::LoadBody()
    {
        // Laod Directories and Files
        GoToChank(m_ChankIndex);
        m_DirectoriesCount = m_FileStream.Read<unsigned char>();
        m_FileStream.Read((char*)m_DirectoriesLocations, MAX_DIRECTORIES);

        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            GoToChank(m_DirectoriesLocations[i]);
            m_DirectoriesNames[i].LoadName(m_FileStream);
        }

        GoToChank(m_ChankIndex, 1 + MAX_DIRECTORIES * 4);
        m_FilesCount = m_FileStream.Read<unsigned char>();
        m_FileStream.Read((char*)m_FilesLocations, MAX_FILES);

        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            GoToChank(m_FilesLocations[i]);
            m_FilesNames[i].LoadName(m_FileStream);
        }
    }

    // Directories Actions
    unsigned int Drive::CreateDirectory(const EntityName& name)
    {
        if (m_DirectoriesCount == MAX_DIRECTORIES)
        {
            Logger::Error("Can't create more then ", MAX_DIRECTORIES, " directories!");
            return 0;
        }

        if (!Drive::CheakEntityName(name))
        {
            return false;
        }

        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            if (m_DirectoriesNames[i].IsEqual(name))
            {
                Logger::Error("This Name already exist!");
                return 0;
            }
        }

        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            if (m_FilesNames[i].IsEqual(name))
            {
                Logger::Error("This Name already exist!");
                return 0;
            }
        }

        unsigned char index = m_DirectoriesCount;
        unsigned int chankIndex = GenerateChank();
        Logger::Info("Drive ", m_DriveName, ": Generate Chank ", chankIndex, " for Directory \"", &name[0], "\"");

        GoToChank(m_ChankIndex);

        m_DirectoriesCount++;
        m_FileStream.Write(m_DirectoriesCount);

        m_DirectoriesLocations[index] = chankIndex;
        m_DirectoriesNames[index].Change(name);
        m_FileStream += index * 4;
        m_FileStream.Write(m_DirectoriesLocations[index]);

        GoToChank(chankIndex);

        m_FileStream.Write(&name[0], MAX_ENTITY_NAME);
        std::array<char, CHANK_SIZE - MAX_ENTITY_NAME> data;
        data.fill(0);
        m_FileStream.Write(&data[0], data.size());

        return chankIndex;
    }

    void Drive::DeleteDirectory(unsigned char directoryIndex)
    {
        unsigned int chankIndex = m_DirectoriesLocations[directoryIndex];

        Directory directory(chankIndex, this);
        directory.Delete();

        DeleteChank(chankIndex);

        m_DirectoriesCount--;

        unsigned char lastIndex = m_DirectoriesCount;
        if (directoryIndex != lastIndex)
        {
            m_DirectoriesLocations[directoryIndex] = m_DirectoriesLocations[lastIndex];
            m_DirectoriesNames[directoryIndex] = m_DirectoriesNames[lastIndex];
        }

        m_DirectoriesLocations[lastIndex] = 0;
        m_DirectoriesNames[lastIndex].Clear();

        GoToChank(m_ChankIndex);
        m_FileStream.Write(m_DirectoriesCount);

        m_FileStream += lastIndex * 4;
        m_FileStream.Write<unsigned int>(0);
    }

    void Drive::DeleteDirectory(const EntityName& name)
    {

    }

    void Drive::RenameDirectory(unsigned char directoryIndex, const EntityName& name)
    {
        unsigned int chankIndex = m_DirectoriesLocations[directoryIndex];
        m_DirectoriesNames[directoryIndex].Change(name);

        GoToChank(chankIndex);
        m_FileStream.Write(&name[0], MAX_ENTITY_NAME);
    }


    // Files Actions
    unsigned int Drive::CreateFile(const EntityName& name, unsigned int size)
    {
        if (m_FilesCount == MAX_FILES)
        {
            Logger::Error("Can't create more then ", MAX_FILES, " files!");
            return 0;
        }

        if (!Drive::CheakEntityName(name))
        {
            return false;
        }

        for (unsigned char i = 0; i < m_DirectoriesCount; i++)
        {
            if (m_DirectoriesNames[i].IsEqual(name))
            {
                Logger::Error("This Name already exist!");
                return 0;
            }
        }

        for (unsigned char i = 0; i < m_FilesCount; i++)
        {
            if (m_FilesNames[i].IsEqual(name))
            {
                Logger::Error("This Name already exist!");
                return 0;
            }
        }

        unsigned int chankIndex = File::Create(this, name, size);

        m_FilesLocations[m_FilesCount] = chankIndex;
        m_FilesNames[m_FilesCount] = name;

        m_FilesCount++;

        GoToChank(m_ChankIndex, 1 + MAX_DIRECTORIES * 4);
        m_FileStream.Write(m_FilesCount);
        m_FileStream += (m_FilesCount - 1) * 4;
        m_FileStream.Write(chankIndex);

        return chankIndex;
    }

    void Drive::DeleteFile(unsigned char fileIndex)
    {
        unsigned int chankIndex = m_FilesLocations[fileIndex];

        File::DeleteFile(this, chankIndex);

        m_FilesCount--;

        unsigned char lastIndex = m_FilesCount;
        if (fileIndex != lastIndex)
        {
            m_FilesLocations[fileIndex] = m_FilesLocations[lastIndex];
            m_FilesNames[fileIndex] = m_FilesNames[lastIndex];
        }

        m_FilesLocations[lastIndex] = 0;
        m_FilesNames[lastIndex].Clear();

        GoToChank(m_ChankIndex, 1 + MAX_DIRECTORIES * 4);
        m_FileStream.Write(m_FilesCount);

        m_FileStream += lastIndex * 4;
        m_FileStream.Write<unsigned int>(0);
    }

    void Drive::DeleteFile(const EntityName& name)
    {

    }

    void Drive::RenameFile(unsigned char fileIndex, const EntityName& name)
    {
        unsigned int chankIndex = m_FilesLocations[fileIndex];
        m_FilesNames[fileIndex].Change(name);

        GoToChank(chankIndex);
        m_FileStream.Write(&name[0], MAX_ENTITY_NAME);
    }
}