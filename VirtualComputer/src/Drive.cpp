#include "pch.h"
#include "Drive.h"

#include "Utils/Directory.h"

#include "DirectoryBody.h"

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
    drivePath += "//drive";
    drivePath += (char)('1' + s_DrivesActives);
    drivePath += DIVER_EXTENSION;
    char driveName = 'A';

    if (!Utils::File::Exists(drivePath.c_str())) // Not exist drives
    {
        CreateDrive();
    }
    else
    {
        do
        {
            s_Drives[s_DrivesActives] = new Drive(drivePath, driveName);
            s_DrivesActives++;

            if (s_DrivesActives == MAX_DRIVES)
            {
                break;
            }

            drivePath = DIVERS_PATH;
            drivePath += "//drive";
            unsigned int driveId = s_DrivesActives + 1;
            if (driveId >= 10)
            {
                drivePath += (char)('0' + (driveId / 10));
            }
            drivePath += (char)('0' + (driveId % 10));
            drivePath += DIVER_EXTENSION;

            driveName++;

        } while (Utils::File::Exists(drivePath.c_str()));
    }

    s_DriveCurrent = s_Drives[0];
}

Drive* Drive::CreateDrive()
{
    if (s_DrivesActives == MAX_DRIVES)
    {
        return nullptr;
    }
    std::string drivePath = DIVERS_PATH;
    drivePath += "//drive";
    unsigned int driveId = s_DrivesActives + 1;
    if (driveId >= 10)
    {
        drivePath += (char)('0' + (driveId / 10));
    }
    drivePath += (char)('0' + (driveId % 10));
    drivePath += DIVER_EXTENSION;

    Utils::File fileStream(drivePath, true);
        
    std::array<char, (CHANK_SIZE - MAX_ENTITY_NAME) + CHANK_SIZE> data; // CHANK_SIZE - MAX_ENTITY_NAME = Drive, CHANK_SIZE = DeletedMemoryList
    data.fill(0);
    fileStream.Write(&data[0], data.size());
    fileStream.Close();

    char driveName = (char)('A' + s_DrivesActives);

    Drive* drive = new Drive(drivePath, driveName);

    s_Drives[s_DrivesActives] = drive;
    s_DrivesActives++;

    return drive;
}

constexpr size_t Drive::ChankToFileIndex(const unsigned int& chankIndex)
{
    if (chankIndex == 0)
        return 0;

    return (CHANK_SIZE * chankIndex) - MAX_ENTITY_NAME;
}

// None-Static
void Drive::GoToChank(unsigned int chankIndex, size_t indexInTheChank)
{
    m_FileStream.ChangeIndex(Drive::ChankToFileIndex(chankIndex) + indexInTheChank);
}

unsigned int Drive::GenerateChank()
{
    if (m_DeletedMemoryList.Index > 0)
    {
        // Give Deleted Chank
        m_DeletedMemoryList.Index--;

        int chankIndex = m_DeletedMemoryList.List[m_DeletedMemoryList.Index];
        m_DeletedMemoryList.List[m_DeletedMemoryList.Index] = 0;
        
        GoToChank(m_DeletedMemoryList.ChankIndex, m_DeletedMemoryList.Index);
        m_FileStream.Write<unsigned int>(0);

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

            GoToChank(m_DeletedMemoryList.ChankIndex);

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

void Drive::DeleteChank(unsigned int chankIndex)
{
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
}

// Body Actions
void Drive::LoadBody()
{
    m_DirectoriesCount = m_FileStream.Read<unsigned char>();
    m_FileStream.Read((char*)m_DirectoriesLocations, MAX_DIRECTORIES);

    unsigned char index = 0;
    unsigned int* locationPtr = m_DirectoriesLocations;
    EntityName* namePtr = m_DirectoriesNames;
    while (index < m_DirectoriesCount)
    {
        if (*locationPtr != 0)
        {
            GoToChank(*locationPtr);
            namePtr->LoadName(m_FileStream);
        }
        else
        {
            namePtr->Clear();
        }
        locationPtr++;
        namePtr++;
    }

    m_FilesCount = m_FileStream.Read<unsigned char>();
    m_FileStream.Read((char*)m_FilesLocations, MAX_FILES);
    
    index = 0;
    locationPtr = m_FilesLocations;
    namePtr = m_FilesNames;
    while (index < m_FilesCount)
    {
        if (*locationPtr != 0)
        {
            GoToChank(*locationPtr);
            namePtr->LoadName(m_FileStream);
        }
        else
        {
            namePtr->Clear();
        }
        locationPtr++;
        namePtr++;
    }
}

void Drive::CreateDirectory(const char name[MAX_ENTITY_NAME + 1])
{
    unsigned char index = 0;
    while (index < MAX_DIRECTORIES && m_DirectoriesLocations[index] != 0)
    {
        if (m_DirectoriesNames[index].IsEqual((char*)name))
        {
            Logger::Error("This Name already exist!");
            return;
        }
        index++;
    }

    if (index == MAX_DIRECTORIES)
    {
        Logger::Error("Can't create Directorie");
    }
    else
    {
        unsigned int chankIndex = GenerateChank();

        GoToChank(m_ChankIndex);

        m_DirectoriesCount++;
        m_FileStream.Write(m_DirectoriesCount);

        m_DirectoriesLocations[index] = chankIndex;
        m_DirectoriesNames[index].Change((char*)name);
        m_FileStream += index * 4;
        m_FileStream.Write(m_DirectoriesLocations[index]);

        GoToChank(chankIndex);

        m_FileStream.Write(name, MAX_ENTITY_NAME);

        std::array<char, CHANK_SIZE - MAX_ENTITY_NAME> data;
        data.fill(0);
        m_FileStream.Write(&data[0], data.size());
    }
}

void Drive::DeleteDirectory(unsigned char directoryIndex)
{

}

void Drive::DeleteDirectory(const char name[MAX_ENTITY_NAME + 1])
{

}