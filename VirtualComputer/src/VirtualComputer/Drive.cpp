#include "pch.h"
#include "Drive.h"

#include "Utils/Directory.h"

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
                HardDrive* hardDrive = new HardDrive(drivePath);
                s_Drives[i] = new Drive(hardDrive, driveName);
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

    Drive* Drive::CreateDrive(const char* name)
    {
        int index;

        if (name == nullptr)
        {
            if (s_DrivesActives == MAX_DRIVES)
            {
                std::cout << "Can't create more the " << MAX_DRIVES << " drives.\n";
                return nullptr;
            }

            index = 0;
            while (s_Drives[index] != nullptr)
            {
                index++;
            }
        }
        else
        {
            index = DriveNameToIndex(name);

            if (index == -1)
            {
                std::cout << "Drive name syntex error!\n";
                return nullptr;
            }

            if (Drive::s_Drives[index] != nullptr)
            {
                std::cout << "The drive exists!\n";
                return nullptr;
            }
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

        HardDrive* hardDrive = new HardDrive(drivePath);
        Drive* drive = new Drive(hardDrive, driveName);

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
            std::cout << "The drive not exists!\n";
            return 0;
        }

        if (drive == Drive::s_DriveCurrent)
        {
            std::cout << "Can't delete the current drive!\n";
            return 0;
        }

        if (drive->m_Drive->Delete())
        {
            delete drive;
            Drive::s_Drives[index] = nullptr;
            s_DrivesActives--;
        }
        else
        {
            return 0;
        }

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

    // None-Static
    void Drive::GoToThisChankBody(size_t indexInTheChank)
    {
        m_Drive->GoToChank(m_ChankIndex, 0 + indexInTheChank);
    }
}