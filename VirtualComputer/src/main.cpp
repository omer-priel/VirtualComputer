#include "pch.h"

#include "Drive.h"
#include "Directory.h"

// Drive = file
// Path = Drive:/DirectoryNames.../FileName

/*
    Names:
        Drive, Directory, File, Path
        Entity = Directory or File.
        Chank = Area of bytes on the drive.
        Location = ChankId
    Settings:
        MAX_DRIVES 26
        MAX_DIRECTORIES = 255
        MAX_FILES = 255
        MAX_ENTITY_NAME = 22
        POINTER sizeof(unsigned int)
        MAX_FILE_SIZE sizeof(unsigned int)
        CHANK_SIZE = Directory Size
    Drive:
        Drive = CHANK_SIZE - MAX_ENTITY_NAME
            like Directory without CHANK_SIZE

        Directory = CHANK_SIZE
            DirectoryName [MAX_ENTITY_NAME]
            DirectoriesCount MAX_DIRECTORIES
            DirectoriesLocations POINTER[MAX_DIRECTORIES]
            FilesCount MAX_FILES
            FilesLocations [MAX_FILES]

        File = CHANK_SIZE
            FileName [MAX_ENTITY_NAME]
            FileSize MAX_FILE_SIZE
            FileBody [CHANK_SIZE - 4]
            FileBody
                do
                    BodyChank [CHANK_SIZE - 4]
                    if (File <= CHANK_SIZE - 4)
                        4 bytes of 0
                    else
                        PointerToNextChank
                while (FileSize <= CHANK_SIZE - 4);
        DeletedMemoryList = CHANK_SIZE
            ChanksPointers [CHANK_SIZE - 4]
            NextDeletedMemoryList ChankLocation
*/

void PrintDirectoryBody(DirectoryBody* directoryBody)
{
    std::cout << "Directories:\n";
    for (unsigned char i = 0; i < directoryBody->m_DirectoriesCount; i++)
    {
        std::cout << directoryBody->m_DirectoriesNames[i].GetName() << "\n";
    }
    std::cout << "\n";

    std::cout << "Files:\n";
    for (unsigned char i = 0; i < directoryBody->m_FilesCount; i++)
    {
        std::cout << directoryBody->m_FilesNames[i].GetName() << "\n";
    }
    std::cout << "\n";
}

void PrintDrive(Drive* drive)
{
    std::cout << "Drive " << drive->m_DriveName << ":\n";
    PrintDirectoryBody(drive);
}

void PrintDirectory(Directory* directory)
{
    std::cout << "Directory " << directory->m_Name.GetName() << ":\n";
    PrintDirectoryBody(directory);
}

EntityName CreateName(const char* name)
{
    EntityName ret;
    ret.fill(0);
    for (size_t i = 0; i < MAX_ENTITY_NAME; i++)
    {
        ret[i] = name[i];
        if (name[i] == 0)
            break;
    }

    ret[MAX_ENTITY_NAME] = 0;
    return ret;
}

Drive* drive;

void Test()
{
    drive = Drive::s_DriveCurrent;
    
    int code;
    std::cin >> code;
    if (code == 1)
    {
        drive->CreateDirectory(CreateName("New Folder"));
    }
    else if (code == 2)
    {
        drive->DeleteDirectory(0);
    }
}

int main()
{
    Utils::Debug::DebugTrace::BeginSession();

    Logger::ChangeLevel(Logger::Level::Info);

    // Load Drives
    bool haveError = false;
    Drive::LoadDrives(haveError);
    
    if (haveError)
    {
        std::cin.get();
        return 1;
    }

    // Start Runing
    for (Drive* drive : Drive::s_Drives)
    {
        if (drive != nullptr)
        {
            PrintDrive(drive);
        }
    }
    
    Test();

    for (Drive* drive : Drive::s_Drives)
    {
        if (drive != nullptr)
        {
            PrintDrive(drive);
        }
    }

    Utils::Debug::DebugTrace::EndSession();
    system("PAUSE");//std::cout << "Press any key to continue . . .";
}