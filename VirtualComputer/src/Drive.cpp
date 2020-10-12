#include "pch.h"
#include "Drive.h"

#include "Utils/Directory.h"

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
        
    std::array<char, (CHANK_SIZE - MAX_ENTITY_NAME) + (MAX_FILES + 4)> arr; // CHANK_SIZE - MAX_ENTITY_NAME = Drive, MAX_FILES + DeletedMemoryList
    arr.fill(0);
    fileStream.Write(&arr[0], arr.size());
    fileStream.Close();

    char driveName = (char)('A' + s_DrivesActives);

    Drive* drive = new Drive(drivePath, driveName);

    s_Drives[s_DrivesActives] = drive;
    s_DrivesActives++;

    return drive;
}