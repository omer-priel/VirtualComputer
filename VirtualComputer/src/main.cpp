#include "pch.h"

#include "Utils/Directory.h"

// Drive = file
// Path = Drive:/DirectoryNames.../FileName

/*
    Names:
        Drive, Directory, File, Path
        Entity = Directory or File.
        Chank = Area of bytes on the drive.
    Settings:
        MAX_DIRECTORIES_AND_FILES = 255
        MAX_ENTITY_NAME = 20
        POINTER sizeof(unsigned int)
        MAX_FILE_SIZE sizeof(unsigned int)
        CHANK_SIZE = Directory Size
    Drive:
        Directory
            DirectoryName [MAX_ENTITY_NAME]
            EntitiesCount
            EntityPointers [MAX_DIRECTORIES_AND_FILES]
        File
            FileName [MAX_ENTITY_NAME]
            FileSize MAX_FILE_SIZE
            FileBody [CHANK_SIZE - 4]
            FileBody
                do
                    BodyChank [CHANK_SIZE - 4]
                    if (FileSize <= CHANK_SIZE - 4)
                        4 bytes of 0
                    else
                        PointerToNextChank
                while (FileSize <= CHANK_SIZE - 4);
*/

int main()
{
    Utils::Debug::DebugTrace::BeginSession();

    bool isCreatedNow;
    Utils::Directory::Exists("drives", true, isCreatedNow);
    if (isCreatedNow)
    {
        std::cout << "Is created now!\n";
    }
    else
    {
        std::cout << "Exists\n";
    }

    std::cout << "Press any key to continue . . .";
    std::cin.get();
    Utils::Debug::DebugTrace::EndSession();
}