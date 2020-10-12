#include "pch.h"

#include "Drive.h"

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

int main()
{
    Utils::Debug::DebugTrace::BeginSession();

    // Load Drives
    bool haveError = false;
    Drive::LoadDrives(haveError);
    
    if (haveError)
    {
        std::cin.get();
        return 1;
    }

    // Start Runing
    // TODO Delete this loop
    for (Drive* drive : Drive::s_Drives)
    {
        if (drive != nullptr)
        {
            std::cout << drive->m_DriveName << ":\n";
        }
    }

    std::cout << "Press any key to continue . . .";
    std::cin.get();
    Utils::Debug::DebugTrace::EndSession();
}