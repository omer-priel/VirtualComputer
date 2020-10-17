#include "pch.h"

#include "Drive.h"
#include "Directory.h"

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
    
    std::cout << "Code: ";
    int code;
    std::cin >> code;
    if (code == 1)
    {
        drive->CreateFile(CreateName("text.txt"), CHANK_SIZE * 6);
    }
    else if (code == 2)
    {
        drive->DeleteFile(0);
    }
    else if (code == 3)
    {
        drive->CreateDirectory(CreateName("New Folder"));
    }
    else if (code == 4)
    {
        drive->DeleteDirectory(0);
    }
}

int main()
{
    Utils::Debug::DebugTrace::BeginSession();
    Logger::ChangeLevel(Logger::Level::Warning);

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